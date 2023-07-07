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
#include <QScopeGuard>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include <QUrl>

#include "appconstants.h"
#include "feature.h"
#include "logger.h"
#include "urlopener.h"

#ifdef MZ_ANDROID
#  include <android/log.h>

#  include "platforms/android/androidcommons.h"
#endif

#ifdef MZ_IOS
#  include "platforms/ios/ioscommons.h"
#  include "platforms/ios/ioslogger.h"
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

// Please! Use this `logger` carefully in this file to avoid log loops!
Logger logger("LogHandler");
#ifdef MZ_IOS
IOSLogger iosLogger("mozillavpn");
#endif

class LogSerializeHelper final {
 public:
  LogSerializeHelper(QTextStream* out,
                     const QList<LogSerializer*>& logSerializers,
                     std::function<void()>&& callback)
      : m_out(out),
        m_logSerializers(logSerializers),
        m_callback(std::move(callback)) {}

  void run() {
    if (m_logSerializers.isEmpty()) {
      m_callback();
      delete this;
      return;
    }

    LogSerializer* logSerializer = m_logSerializers.takeFirst();
    logSerializer->serializeLogs(
        [this](const QString& name, const QString& logs) {
          *m_out << Qt::endl
                 << Qt::endl
                 << name << Qt::endl
                 << QByteArray(name.length(), '=') << Qt::endl
                 << Qt::endl;

          if (!logs.isEmpty()) {
            *m_out << logs;
          } else {
            *m_out << "No logs.";
          }

          run();
        });
  }

 private:
  QTextStream* m_out;
  QList<LogSerializer*> m_logSerializers;
  std::function<void()> m_callback;
};

}  // namespace

// static
LogHandler* LogHandler::instance() {
  QMutexLocker<QMutex> lock(&s_mutex);
  return maybeCreate(lock);
}

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& message) {
  QMutexLocker<QMutex> lock(&s_mutex);
  maybeCreate(lock)->addLog(Log(qtTypeToLogLevel(type), context.file,
                                context.function, context.line, message),
                            lock);
}

// static
void LogHandler::messageHandler(LogLevel logLevel, const QString& className,
                                const QString& message) {
  QMutexLocker<QMutex> lock(&s_mutex);
  maybeCreate(lock)->addLog(Log(logLevel, className, message), lock);
}

// static
void LogHandler::rustMessageHandler(int32_t logLevel, char* message) {
  QMutexLocker<QMutex> lock(&s_mutex);

  maybeCreate(lock)->addLog(
      Log(static_cast<LogLevel>(logLevel), "Rust", QString::fromUtf8(message)),
      lock);
}

// static
LogHandler* LogHandler::maybeCreate(const QMutexLocker<QMutex>& proofOfLock) {
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
void LogHandler::setStderr(bool enabled) {
  QMutexLocker<QMutex> lock(&s_mutex);
  maybeCreate(lock)->m_stderrEnabled = enabled;
}

LogHandler::LogHandler(const QMutexLocker<QMutex>& proofOfLock) {
  Q_UNUSED(proofOfLock);

#if defined(MZ_DEBUG)
  m_stderrEnabled = true;
#endif

  if (!s_location.isEmpty()) {
    openLogFile(proofOfLock);
  }
}

void LogHandler::addLog(const Log& log,
                        const QMutexLocker<QMutex>& proofOfLock) {
  if (m_output) {
    prettyOutput(*m_output, log);
  }

  QByteArray buffer;
  {
    QTextStream out(&buffer);
    prettyOutput(out, log);
  }

  if (m_stderrEnabled) {
#if defined(MZ_IOS)
    switch (log.m_logLevel) {
      case Error:
      case Warning:
        IOSLogger::error(buffer);
        break;
      case Info:
        IOSLogger::info(buffer);
        break;
      default:
        IOSLogger::debug(buffer);
        break;
    }
#else
    QTextStream out(stderr);
    prettyOutput(out, log);
#endif
  }

  emit logEntryAdded(buffer);

#if defined(MZ_ANDROID)
  if (!Constants::inProduction()) {
    const char* str = buffer.constData();
    if (str) {
      __android_log_write(ANDROID_LOG_DEBUG, AppConstants::ANDROID_LOG_NAME,
                          str);
    }
  }
#endif
}

// static
void LogHandler::writeLogs(QTextStream& out) {
  QMutexLocker<QMutex> lock(&s_mutex);

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
  QMutexLocker<QMutex> lock(&s_mutex);
  cleanupLogFile(lock);
}

// static
void LogHandler::cleanupLogFile(const QMutexLocker<QMutex>& proofOfLock) {
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
  QMutexLocker<QMutex> lock(&s_mutex);
  s_location = path;

  if (s_instance && s_instance->m_logFile) {
    cleanupLogFile(lock);
  }
}

// static
void LogHandler::truncateLogFile(const QMutexLocker<QMutex>& proofOfLock,
                                 const QString& filename) {
  QFile oldLogFile(filename);

  // Nothing to do if the log file is already undersize.
  if (!oldLogFile.exists()) {
    return;
  }
  if (oldLogFile.size() < LOG_MAX_FILE_SIZE) {
    return;
  }

  // Rename the old log file while we truncate and ensure it gets cleaned up.
  auto guard = qScopeGuard([&] { oldLogFile.remove(); });
  if (!oldLogFile.rename(filename + ".bak")) {
    return;
  }

  // Discard all but the tail of the log and align to the next new line.
  if (!oldLogFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }
  oldLogFile.seek(oldLogFile.size() - (LOG_MAX_FILE_SIZE / 2));
  oldLogFile.readLine();

  // Re-create the original log file and copy the truncated contents.
  QFile newLogFile(filename);
  if (newLogFile.open(QIODevice::WriteOnly | QIODevice::Truncate |
                      QIODevice::Text)) {
    newLogFile.write(oldLogFile.readAll());
  }
}

void LogHandler::openLogFile(const QMutexLocker<QMutex>& proofOfLock) {
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
  truncateLogFile(proofOfLock, logFileName);

  m_logFile = new QFile(logFileName);
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

void LogHandler::closeLogFile(const QMutexLocker<QMutex>& proofOfLock) {
  Q_UNUSED(proofOfLock);

  if (m_logFile) {
    delete m_output;
    m_output = nullptr;

    delete m_logFile;
    m_logFile = nullptr;
  }
}

bool LogHandler::viewLogs() {
  logger.debug() << "View logs";

  if (!Feature::get(Feature::Feature_shareLogs)->isSupported()) {
    logger.error() << "ViewLogs Called on unsupported OS or version!";
    return false;
  }

#if defined(MZ_ANDROID) || defined(MZ_IOS)
  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);
  bool ok = true;
  serializeLogs(out, [buffer, out
#  if defined(MZ_ANDROID)
                      ,
                      &ok
#  endif
  ]() {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

#  if defined(MZ_ANDROID)
    ok = AndroidCommons::shareText(*buffer);
#  else
    IOSCommons::shareLogs(*buffer);
#  endif

    delete out;
    delete buffer;
  });
  return ok;
#endif

  if (writeAndShowLogs(QStandardPaths::DesktopLocation)) {
    return true;
  }

  if (writeAndShowLogs(QStandardPaths::HomeLocation)) {
    return true;
  }

  if (writeAndShowLogs(QStandardPaths::TempLocation)) {
    return true;
  }

  logger.warning()
      << "No Desktop, no Home, no Temp folder. Unable to store the log files.";
  return false;
}

void LogHandler::requestViewLogs() {
  logger.debug() << "View log requested";
  emit viewLogsNeeded();
}

void LogHandler::retrieveLogs() {
  logger.debug() << "Retrieve logs";

  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);

  serializeLogs(out, [this, buffer, out]() {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

    delete out;
    emit logsReady(*buffer);
    delete buffer;
  });
}

void LogHandler::serializeLogs(QTextStream* out,
                               std::function<void()>&& a_finalizeCallback) {
  std::function<void()> finalizeCallback = std::move(a_finalizeCallback);

  *out << "MZ logs" << Qt::endl << "=======" << Qt::endl << Qt::endl;

  LogHandler::writeLogs(*out);

  LogSerializeHelper* lsh = new LogSerializeHelper(out, m_logSerializers,
                                                   std::move(finalizeCallback));
  lsh->run();
}

bool LogHandler::writeLogsToLocation(
    QStandardPaths::StandardLocation location,
    std::function<void(const QString& filename)>&& a_callback) {
  logger.debug() << "Trying to save logs in:" << location;

  std::function<void(const QString& filename)> callback = std::move(a_callback);

  if (!QFileInfo::exists(QStandardPaths::writableLocation(location))) {
    return false;
  }

  QString filename;
  QDate now = QDate::currentDate();

  QFileInfo logFileInfo(AppConstants::LOG_FILE_NAME);

  QTextStream(&filename) << logFileInfo.baseName() << "-" << now.year() << "-"
                         << now.month() << "-" << now.day() << "."
                         << logFileInfo.completeSuffix();

  QDir logDir(QStandardPaths::writableLocation(location));
  QString logFile = logDir.filePath(filename);

  if (QFileInfo::exists(logFile)) {
    logger.warning() << logFile << "exists. Let's try a new filename";

    for (uint32_t i = 1;; ++i) {
      QString filename;
      QTextStream(&filename) << logFileInfo.baseName() << "-" << now.year()
                             << "-" << now.month() << "-" << now.day() << "_"
                             << i << "." << logFileInfo.completeSuffix();
      logFile = logDir.filePath(filename);
      if (!QFileInfo::exists(logFile)) {
        logger.debug() << "Filename found!" << i;
        break;
      }
    }
  }

  logger.debug() << "Writing logs into: " << logFile;

  QFile* file = new QFile(logFile);
  if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
    logger.error() << "Failed to open the logfile";
    delete file;
    return false;
  }

  QTextStream* out = new QTextStream(file);
  serializeLogs(out, [callback = std::move(callback), logFile, file, out]() {
    Q_ASSERT(out);
    Q_ASSERT(file);
    delete out;
    delete file;

    callback(logFile);
  });

  return true;
}

bool LogHandler::writeAndShowLogs(QStandardPaths::StandardLocation location) {
  return writeLogsToLocation(location, [](const QString& filename) {
    logger.debug() << "Opening the logFile somehow:" << filename;
    UrlOpener::instance()->openUrl(QUrl::fromLocalFile(filename));
  });
}

void LogHandler::flushLogs() {
  logger.debug() << "Cleanup logs";
  cleanupLogs();
  emit cleanupLogsNeeded();
}

void LogHandler::registerLogSerializer(LogSerializer* logSerializer) {
  Q_ASSERT(!m_logSerializers.contains(logSerializer));
  m_logSerializers.append(logSerializer);
}

void LogHandler::unregisterLogSerializer(LogSerializer* logSerializer) {
  Q_ASSERT(m_logSerializers.contains(logSerializer));
  m_logSerializers.removeOne(logSerializer);
}
