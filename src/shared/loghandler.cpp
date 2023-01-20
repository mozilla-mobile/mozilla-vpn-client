/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loghandler.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#include "appconstants.h"

#ifdef MZ_ANDROID
#  include <android/log.h>
#endif

constexpr qint64 LOG_MAX_FILE_SIZE = 204800;

namespace {
QMutex s_mutex;
QString s_location =
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
LogHandler* s_instance = nullptr;

LogLevel qtTypeToLogLevel(QtMsgType type) {
  switch (type) {
    case QtDebugMsg:
      return Debug;
    case QtInfoMsg:
      return Info;
    case QtWarningMsg:
      return Warning;
    case QtCriticalMsg:
      [[fallthrough]];
    case QtFatalMsg:
      return Error;
    default:
      return Debug;
  }
}

}  // namespace

// static
LogHandler* LogHandler::instance() {
  MutexLocker lock(&s_mutex);
  return maybeCreate(lock);
}

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& message) {
  MutexLocker lock(&s_mutex);
  maybeCreate(lock)->addLog(Log(qtTypeToLogLevel(type), context.file,
                                context.function, context.line, message),
                            lock);
}

// static
void LogHandler::messageHandler(LogLevel logLevel, const QString& className,
                                const QString& message) {
  MutexLocker lock(&s_mutex);
  maybeCreate(lock)->addLog(Log(logLevel, className, message), lock);
}

// static
void LogHandler::rustMessageHandler(int32_t logLevel, char* message) {
  MutexLocker lock(&s_mutex);

  maybeCreate(lock)->addLog(
      Log(static_cast<LogLevel>(logLevel), "Rust", QString::fromUtf8(message)),
      lock);
}

// static
LogHandler* LogHandler::maybeCreate(const MutexLocker& proofOfLock) {
  if (!s_instance) {
    s_instance = new LogHandler(proofOfLock);
  }

  return s_instance;
}

// static
void LogHandler::prettyOutput(QTextStream& out, const LogHandler::Log& log) {
  out << "[" << log.m_dateTime.toString("dd.MM.yyyy hh:mm:ss.zzz") << "] ";

  if (!log.m_className.isEmpty()) {
    out << "(" << log.m_className << ") ";
  }

  switch (log.m_logLevel) {
    case Trace:
      out << "Trace: ";
      break;
    case Debug:
      out << "Debug: ";
      break;
    case Info:
      out << "Info: ";
      break;
    case Warning:
      out << "Warning: ";
      break;
    case Error:
      out << "Error: ";
      break;
    default:
      out << "?!?: ";
      break;
  }

  if (log.m_fromQT) {
    out << log.m_message;

    if (!log.m_file.isEmpty() || !log.m_function.isEmpty()) {
      out << " (";

      if (!log.m_file.isEmpty()) {
        qsizetype pos = log.m_file.lastIndexOf("/");
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
    out << log.m_message;
  }

  out << Qt::endl;
}

// static
void LogHandler::enableStderr() {
  MutexLocker lock(&s_mutex);
  maybeCreate(lock)->m_stderrEnabled = true;
}

LogHandler::LogHandler(const MutexLocker& proofOfLock) {
  Q_UNUSED(proofOfLock);

#if defined(MZ_DEBUG)
  m_stderrEnabled = true;
#endif

  if (!s_location.isEmpty()) {
    openLogFile(proofOfLock);
  }
}

void LogHandler::addLog(const Log& log, const MutexLocker& proofOfLock) {
  if (m_output) {
    prettyOutput(*m_output, log);
  }

  if (m_stderrEnabled) {
    QTextStream out(stderr);
    prettyOutput(out, log);
  }

  QByteArray buffer;
  {
    QTextStream out(&buffer);
    prettyOutput(out, log);
  }

  emit logEntryAdded(buffer);

#if defined(MZ_ANDROID) && defined(MZ_DEBUG)
  const char* str = buffer.constData();
  if (str) {
    __android_log_write(ANDROID_LOG_DEBUG, AppConstants::ANDROID_LOG_NAME, str);
  }
#endif
}

// static
void LogHandler::writeLogs(QTextStream& out) {
  MutexLocker lock(&s_mutex);

  if (!s_instance || !s_instance->m_logFile) {
    return;
  }

  QString logFileName = s_instance->m_logFile->fileName();
  s_instance->closeLogFile(lock);

  {
    QFile file(logFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;
    }

    out << file.readAll();
  }

  s_instance->openLogFile(lock);
}

// static
void LogHandler::cleanupLogs() {
  MutexLocker lock(&s_mutex);
  cleanupLogFile(lock);
}

// static
void LogHandler::cleanupLogFile(const MutexLocker& proofOfLock) {
  if (!s_instance || !s_instance->m_logFile) {
    return;
  }

  QString logFileName = s_instance->m_logFile->fileName();
  s_instance->closeLogFile(proofOfLock);

  {
    QFile file(logFileName);
    file.remove();
  }

  s_instance->openLogFile(proofOfLock);
}

// static
void LogHandler::setLocation(const QString& path) {
  MutexLocker lock(&s_mutex);
  s_location = path;

  if (s_instance && s_instance->m_logFile) {
    cleanupLogFile(lock);
  }
}

void LogHandler::openLogFile(const MutexLocker& proofOfLock) {
  Q_UNUSED(proofOfLock);
  Q_ASSERT(!m_logFile);
  Q_ASSERT(!m_output);

  QDir appDataLocation(s_location);
  if (!appDataLocation.exists()) {
    QDir tmp(s_location);
    tmp.cdUp();
    if (!tmp.exists()) {
      return;
    }
    if (!tmp.mkdir(appDataLocation.dirName())) {
      return;
    }
  }

  QString logFileName = appDataLocation.filePath(AppConstants::LOG_FILE_NAME);
  m_logFile = new QFile(logFileName);
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

  addLog(Log(Debug, "LogHandler", QString("Log file: %1").arg(logFileName)),
         proofOfLock);
}

void LogHandler::closeLogFile(const MutexLocker& proofOfLock) {
  Q_UNUSED(proofOfLock);

  if (m_logFile) {
    delete m_output;
    m_output = nullptr;

    delete m_logFile;
    m_logFile = nullptr;
  }
}
