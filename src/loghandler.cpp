/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loghandler.h"
#include "constants.h"
#include "logger.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#ifdef MVPN_ANDROID
#  include <android/log.h>
#endif

constexpr qint64 LOG_MAX_FILE_SIZE = 204800;
constexpr const char* LOG_FILENAME = "mozillavpn.txt";

using namespace std;

namespace {

QString s_location =
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

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
LogHandler& LogHandler::instance() {
  static LogHandler instance;
  return instance;
}

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& message) {
  auto& handler = instance();
  lock_guard<recursive_mutex> lock(handler.m_mutex);
  handler.addLog(Log(qtTypeToLogLevel(type), context.file, context.function,
                     context.line, message));
}

// static
void LogHandler::messageHandler(LogLevel logLevel, const QStringList& modules,
                                const QString& className,
                                const QString& message) {
  auto& handler = instance();
  lock_guard<recursive_mutex> lock(handler.m_mutex);
  handler.addLog(Log(logLevel, modules, className, message));
}

// static
void LogHandler::prettyOutput(QTextStream& out, const LogHandler::Log& log) {
  out << "[" << log.m_dateTime.toString("dd.MM.yyyy hh:mm:ss.zzz") << "] ";

  switch (log.m_logLevel) {
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

// static
void LogHandler::enableDebug() { instance().m_showDebug = true; }

LogHandler::LogHandler() {
  lock_guard<recursive_mutex> lock(m_mutex);
  m_minLogLevel = Debug;  // TODO: in prod, we should log >= warning
  QStringList modules;
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains("MOZVPN_LEVEL")) {
    QString level = pe.value("MOZVPN_LEVEL");
    if (level == "info")
      m_minLogLevel = Info;
    else if (level == "warning")
      m_minLogLevel = Warning;
    else if (level == "error")
      m_minLogLevel = Error;
  }

  if (pe.contains("MOZVPN_LOG")) {
    QStringList parts = pe.value("MOZVPN_LOG").split(",");
    for (const QString& part : parts) {
      m_modules.append(part.trimmed());
    }
  }

#if defined(MVPN_DEBUG) || defined(MVPN_WASM)
  m_showDebug = true;
#endif

  if (!s_location.isEmpty()) {
    openLogFile();
  }
}

void LogHandler::addLog(const Log& log) {
  if (!matchLogLevel(log)) {
    return;
  }

  if (!matchModule(log)) {
    return;
  }

  if (m_output) {
    prettyOutput(*m_output, log);
  }

  if ((log.m_logLevel != LogLevel::Debug) || m_showDebug) {
    QTextStream out(stderr);
    prettyOutput(out, log);
  }

  QByteArray buffer;
  {
    QTextStream out(&buffer);
    prettyOutput(out, log);
  }

  emit logEntryAdded(buffer);

#if defined(MVPN_ANDROID) && defined(MVPN_DEBUG)
  const char* str = buffer.constData();
  if (str) {
    __android_log_write(ANDROID_LOG_DEBUG, "mozillavpn", str);
  }
#endif
}

bool LogHandler::matchModule(const Log& log) const {
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

bool LogHandler::matchLogLevel(const Log& log) const {
  return log.m_logLevel >= m_minLogLevel;
}

// static
void LogHandler::writeLogs(QTextStream& out) {
  auto& handler = instance();
  lock_guard<recursive_mutex> lock(handler.m_mutex);
  if (!handler.m_logFile) {
    return;
  }

  QString logFileName = handler.m_logFile->fileName();
  handler.closeLogFile();

  {
    QFile file(logFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;
    }

    out << file.readAll();
  }

  handler.openLogFile();
}

// static
void LogHandler::cleanupLogs() { cleanupLogFile(); }

// static
void LogHandler::cleanupLogFile() {
  auto& handler = instance();
  if (!handler.m_logFile) {
    return;
  }
  lock_guard<recursive_mutex> lock(handler.m_mutex);
  QString logFileName = handler.m_logFile->fileName();
  handler.closeLogFile();

  {
    QFile file(logFileName);
    file.remove();
  }

  handler.openLogFile();
}

// static
void LogHandler::setLocation(const QString& path) {
  auto& handler = instance();
  lock_guard<recursive_mutex> lock(handler.m_mutex);
  s_location = path;

  if (handler.m_logFile) {
    cleanupLogFile();
  }
}

void LogHandler::openLogFile() {
  Q_ASSERT(!m_logFile);
  Q_ASSERT(!m_output);
  lock_guard<recursive_mutex> lock(m_mutex);
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

  QString logFileName = appDataLocation.filePath(LOG_FILENAME);
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

  addLog(Log(Debug, QStringList{LOG_MAIN}, "LogHandler",
             QString("Log file: %1").arg(logFileName)));
}

void LogHandler::closeLogFile() {
  lock_guard<recursive_mutex> lock(m_mutex);
  if (m_logFile) {
    delete m_output;
    m_output = nullptr;

    delete m_logFile;
    m_logFile = nullptr;
  }
}
