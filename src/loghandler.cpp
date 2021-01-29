/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loghandler.h"
#include "logger.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QMutexLocker>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#ifdef MVPN_ANDROID
#  include <android/log.h>
#endif

constexpr qint64 LOG_MAX_FILE_SIZE = 204800;
constexpr const char* LOG_FILENAME = "mozillavpn.txt";

namespace {
QMutex s_mutex;
LogHandler* s_instance = nullptr;
}  // namespace

// static
LogHandler* LogHandler::instance() {
  QMutexLocker lock(&s_mutex);
  return maybeCreate(lock);
}

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& message) {
  QMutexLocker lock(&s_mutex);
  maybeCreate(lock)->addLog(
      Log(type, context.file, context.function, context.line, message), lock);
}

// static
void LogHandler::messageHandler(const QStringList& modules,
                                const QString& className,
                                const QString& message) {
  QMutexLocker lock(&s_mutex);
  maybeCreate(lock)->addLog(Log(modules, className, message), lock);
}

// static
LogHandler* LogHandler::maybeCreate(const QMutexLocker& proofOfLock) {
  if (!s_instance) {
    QStringList modules;
    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
    if (pe.contains("MOZVPN_LOG")) {
      QStringList parts = pe.value("MOZVPN_LOG").split(",");
      for (const QString& part : parts) {
        modules.append(part.trimmed());
      }
    }

    s_instance = new LogHandler(modules, proofOfLock);
  }

  return s_instance;
}

// static
void LogHandler::prettyOutput(QTextStream& out, const LogHandler::Log& log) {
  out << "[" << log.m_dateTime.toString("dd.MM.yyyy hh:mm:ss.zzz") << "] ";

  if (log.m_fromQT) {
    switch (log.m_type) {
      case QtDebugMsg:
        out << "Debug: ";
        break;
      case QtInfoMsg:
        out << "Info: ";
        break;
      case QtWarningMsg:
        out << "Warning: ";
        break;
      case QtCriticalMsg:
        out << "Critical: ";
        break;
      case QtFatalMsg:
        out << "Fatal: ";
        break;
      default:
        out << "?!?: ";
        break;
    }

    out << log.m_message;

    if (!log.m_file.isEmpty() || !log.m_function.isEmpty()) {
      out << " (";

      if (!log.m_file.isEmpty()) {
        int pos = log.m_file.lastIndexOf("/");
        out << log.m_file.right(log.m_file.length() - pos - 1);

        if (log.m_line >= 0) {
          out << ":" << log.m_line;
        }

        if (!log.m_function.isEmpty()) {
          out << ", ";
        }
      }

      if (!log.m_function.isEmpty()) {
        out << log.m_function;
      }

      out << ")";
    }
  } else {
    out << "(" << log.m_modules.join("|") << " - " << log.m_className << ") "
        << log.m_message;
  }

  out << Qt::endl;
}

LogHandler::LogHandler(const QStringList& modules,
                       const QMutexLocker& proofOfLock)
    : m_modules(modules) {
  Q_UNUSED(proofOfLock);

  QString location =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (!location.isEmpty()) {
    QDir appDataLocation(location);
    if (!QFileInfo::exists(location)) {
      QDir tmp(location);
      tmp.cdUp();
      if (tmp.exists()) {
        tmp.mkdir(appDataLocation.dirName());
      }
    }

    if (QFileInfo::exists(location)) {
      m_logFileName = appDataLocation.filePath(LOG_FILENAME);
      openLogFile(proofOfLock);
    }
  }
}

void LogHandler::addLog(const Log& log, const QMutexLocker& proofOfLock) {
  if (!matchModule(log, proofOfLock)) {
    return;
  }

  if (m_output) {
    prettyOutput(*m_output, log);
  }

#if defined(MVPN_ANDROID)
  QByteArray buffer;
  QTextStream out(&buffer);
  prettyOutput(out, log);

  const char* str = buffer.constData();
  if (str) {
    __android_log_write(ANDROID_LOG_DEBUG, "mozillavpn", str);
  }
#elif defined(QT_DEBUG) || defined(MVPN_IOS)
  QTextStream out(stderr);
  prettyOutput(out, log);
#endif
}

bool LogHandler::matchModule(const Log& log,
                             const QMutexLocker& proofOfLock) const {
  Q_UNUSED(proofOfLock);

  // Let's include QT logs always.
  if (log.m_fromQT) {
    return true;
  }

  // If no modules has been specified, let's include all.
  if (m_modules.isEmpty()) {
    return true;
  }

  for (const QString& module : log.m_modules) {
    if (m_modules.contains(module)) {
      return true;
    }
  }

  return false;
}

// static
void LogHandler::writeLogs(QTextStream& out) {
  QMutexLocker lock(&s_mutex);

  if (!s_instance || s_instance->m_logFileName.isEmpty()) {
    return;
  }

  s_instance->closeLogFile(lock);

  {
    QFile file(s_instance->m_logFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;
    }

    out << file.readAll();
  }

  s_instance->openLogFile(lock);
}

// static
void LogHandler::cleanupLogs() {
  QMutexLocker lock(&s_mutex);

  if (!s_instance || s_instance->m_logFileName.isEmpty()) {
    return;
  }

  s_instance->closeLogFile(lock);

  {
    QFile file(s_instance->m_logFileName);
    file.remove();
  }

  s_instance->openLogFile(lock);
}

void LogHandler::openLogFile(const QMutexLocker& proofOfLock) {
  Q_UNUSED(proofOfLock);
  Q_ASSERT(!m_logFile);
  Q_ASSERT(!m_output);

  m_logFile = new QFile(m_logFileName);
  if (m_logFile->size() > LOG_MAX_FILE_SIZE) {
    m_logFile->remove();
  }

  if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append |
                       QIODevice::Text)) {
    delete m_logFile;
    m_logFile = nullptr;
    return;
  }

  m_output = new QTextStream(m_logFile);

  addLog(Log(QStringList{LOG_MAIN}, "LogHandler",
             QString("Log file: %1").arg(m_logFileName)),
         proofOfLock);
}

void LogHandler::closeLogFile(const QMutexLocker& proofOfLock) {
  Q_UNUSED(proofOfLock);

  if (m_logFile) {
    delete m_output;
    m_output = nullptr;

    delete m_logFile;
    m_logFile = nullptr;
  }
}
