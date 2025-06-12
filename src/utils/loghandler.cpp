/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loghandler.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#if defined(MZ_ANDROID)
#  include <android/log.h>
#elif defined(MZ_IOS)
#  include <CoreFoundation/CoreFoundation.h>
#  include <os/log.h>
#endif

#include "logger.h"

constexpr qint64 LOG_MAX_FILE_SIZE = 204800;
constexpr const char* LOG_FILE_SUFFIX = ".log";

namespace {
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
}  // namespace

Q_GLOBAL_STATIC(LogHandler, logHandler);
LogHandler* LogHandler::instance() { return logHandler; }

QString LogHandler::s_filename;

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext& context,
                                  const QString& message) {
  logHandler->addLog(Log(qtTypeToLogLevel(type), context.file, context.function,
                         context.line, message));
}

// static
void LogHandler::messageHandler(LogLevel logLevel, const QString& className,
                                const QString& message) {
  logHandler->addLog(Log(logLevel, className, message));
}

// static
void LogHandler::rustMessageHandler(int32_t logLevel, char* message) {
  logHandler->addLog(
      Log(static_cast<LogLevel>(logLevel), "Rust", QString::fromUtf8(message)));
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

void LogHandler::setStderr(bool enabled) {
  QMutexLocker<QMutex> lock(&m_mutex);
  m_stderrEnabled = enabled;
}

LogHandler::LogHandler() : QObject(nullptr) {
  QMutexLocker<QMutex> lock(&m_mutex);

  QRegularExpression nonAlpha("[^a-z]");
  m_shortname = qApp->applicationName().toLower().remove(nonAlpha);

#if defined(MZ_DEBUG)
  m_stderrEnabled = true;
#endif

#if defined(MZ_IOS)
  CFBundleRef bundle = CFBundleGetMainBundle();
  QString bundleId;
  if (bundle) {
    bundleId = QString::fromNSString((NSString*)CFBundleGetIdentifier(bundle));
  } else {
    bundleId = m_shortname;
  }
  m_ioslog = os_log_create(qPrintable(bundleId), qPrintable(m_shortname));
#endif

  // If no logfile has been set, derive it automatically from the shortname.
  if (s_filename.isEmpty()) {
    QString where =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    s_filename = QDir(where).filePath(m_shortname + LOG_FILE_SUFFIX);
  }
  openLogFile(lock);
}

void LogHandler::addLog(const Log& log) {
  QMutexLocker<QMutex> lock(&m_mutex);
  return addLog(log, lock);
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
#if defined(MZ_ANDROID)
    const char* str = buffer.constData();
    if (str) {
      __android_log_write(ANDROID_LOG_DEBUG, qPrintable(m_shortname), str);
    }
#elif defined(MZ_IOS)
    QString logstr = QString::fromUtf8(buffer);
    switch (log.m_logLevel) {
      case Error:
      case Warning:
        os_log_error(m_ioslog, "%s", qPrintable(logstr));
        break;
      case Info:
        os_log_info(m_ioslog, "%s", qPrintable(logstr));
        break;
      default:
        os_log_debug(m_ioslog, "%s", qPrintable(logstr));
        break;
    }
#else
    QTextStream out(stderr);
    prettyOutput(out, log);
#endif
  }

  emit logEntryAdded(buffer);
}

void LogHandler::writeLogs(QTextStream& out) {
  QMutexLocker<QMutex> lock(&m_mutex);
  if (!m_logFile) {
    return;
  }

  if (m_logFile->seek(0)) {
    out << m_logFile->readAll();

    // Re-open the log file after reading.
    closeLogFile(lock);
    openLogFile(lock);
  }
}

void LogHandler::cleanupLogs() {
  QMutexLocker<QMutex> lock(&m_mutex);
  cleanupLogFile(lock);
}

void LogHandler::cleanupLogFile(const QMutexLocker<QMutex>& proofOfLock) {
  if (!m_logFile) {
    return;
  }
  m_logFile->close();
  m_logFile->remove();

  closeLogFile(proofOfLock);
  openLogFile(proofOfLock);
}

// static
void LogHandler::setLogfile(const QString& path) {
  if (!logHandler.exists()) {
    s_filename = path;
    return;
  }

  QMutexLocker<QMutex> lock(&logHandler->m_mutex);
  s_filename = path;
  if (logHandler->m_logFile) {
    logHandler->cleanupLogFile(lock);
  }
}

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

// static
bool LogHandler::makeLogDir(const QDir& dir) {
  if (dir.exists()) {
    return true;
  }

  // Recursively create the parent.
  QDir parent(QDir::cleanPath(dir.absoluteFilePath("..")));
  if (!makeLogDir(parent)) {
    return false;
  }

  // Create the log directory.
#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  QFileDevice::Permissions perms =
      QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
      QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther |
      QFileDevice::ExeOther;
  return parent.mkdir(dir.dirName(), perms);
#else
  return parent.mkdir(dir.dirName());
#endif
}

void LogHandler::openLogFile(const QMutexLocker<QMutex>& proofOfLock) {
  Q_UNUSED(proofOfLock);
  Q_ASSERT(!m_logFile);
  Q_ASSERT(!m_output);

  QDir appDataLocation = QFileInfo(s_filename).dir();
  if (!makeLogDir(appDataLocation)) {
    return;
  }

  truncateLogFile(proofOfLock, s_filename);

  m_logFile = new QFile(s_filename);
  if (!m_logFile->open(QIODevice::ReadWrite | QIODevice::Append |
                       QIODevice::Text)) {
    delete m_logFile;
    m_logFile = nullptr;
    return;
  }

  m_output = new QTextStream(m_logFile);

#ifdef MZ_DEBUG
  addLog(Log(Debug, "LogHandler", QString("Log file: %1").arg(s_filename)),
         proofOfLock);
#endif
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

void LogHandler::requestViewLogs() {
  logger.debug() << "View log requested";
  emit viewLogsNeeded();
}

void LogHandler::retrieveLogs() {
  logger.debug() << "Retrieve logs";

  QBuffer* buffer = new QBuffer();
  buffer->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
  connect(buffer, &QIODevice::aboutToClose, this, [&]() {
    emit logsReady(buffer->data());
    buffer->deleteLater();
  });
  logSerialize(buffer);
}

void LogHandler::logSerialize(QIODevice* device) {
  QTextStream out(device);
  out << "MZ logs" << Qt::endl << "=======" << Qt::endl << Qt::endl;
  LogHandler::writeLogs(out);
  out.flush();

  LogSerializeHelper* lsh = new LogSerializeHelper(device);
  for (LogSerializer* serializer : m_logSerializers) {
    lsh->addSerializer(serializer);
  }
  lsh->run(device);
}

bool LogHandler::writeLogsToLocation(
    QStandardPaths::StandardLocation location,
    std::function<void(const QString& filename)>&& a_callback) {
#ifdef MZ_DEBUG
  logger.debug() << "Trying to save logs in:" << location;
#else
  logger.debug() << "Trying to save logs.";
#endif

  std::function<void(const QString& filename)> callback = std::move(a_callback);

  if (!QFileInfo::exists(QStandardPaths::writableLocation(location))) {
    return false;
  }

  QString filename;
  QDate now = QDate::currentDate();

  QTextStream(&filename) << m_shortname << "-" << now.year() << "-"
                         << now.month() << "-" << now.day() << LOG_FILE_SUFFIX;

  QDir logDir(QStandardPaths::writableLocation(location));
  QString logFile = logDir.filePath(filename);

  if (QFileInfo::exists(logFile)) {
#ifdef MZ_DEBUG
    logger.warning() << logFile << "exists. Let's try a new filename";
#else
    logger.warning() << "Logfile exists. Let's try a new filename";
#endif

    for (uint32_t i = 1;; ++i) {
      QString filename;
      QTextStream(&filename)
          << m_shortname << "-" << now.year() << "-" << now.month() << "-"
          << now.day() << "_" << i << LOG_FILE_SUFFIX;
      logFile = logDir.filePath(filename);
      if (!QFileInfo::exists(logFile)) {
        logger.debug() << "Filename found!" << i;
        break;
      }
    }
  }

  logger.debug() << "Writing logs.";

  QFile* file = new QFile(logFile);
  if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
    logger.error() << "Failed to open the logfile";
    delete file;
    return false;
  }

  // Serialize!
  connect(file, &QIODevice::aboutToClose, this,
          [callback, logFile]() { callback(logFile); });
  logSerialize(file);
  return true;
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

void LogSerializeHelper::addSerializer(LogSerializer* serializer) {
  // Create a buffer to receive the log data.
  QBuffer* buffer = new QBuffer(this);
  m_buffers.append(buffer);
  buffer->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

  // Write the header to the buffer.
  QTextStream stream(buffer);
  QString name = serializer->logName();
  stream << Qt::endl << Qt::endl << name << Qt::endl;
  stream << QByteArray(name.length(), '=') << Qt::endl << Qt::endl;
  stream.flush();

  // Serialize the logs to the buffer
  serializer->logSerialize(buffer);
}

void LogSerializeHelper::run(QIODevice* device) {
  // Serialize buffers until we encounter one that is still open.
  while (!m_buffers.isEmpty()) {
    QBuffer* buffer = m_buffers.takeFirst();
    // If the buffer is closed - write it immediately.
    if (!buffer->isOpen()) {
      device->write(buffer->data());
      buffer->deleteLater();
      continue;
    }

    // Otherwise wait for it to finish and try again.
    connect(buffer, &QIODevice::aboutToClose, this, [this, device, buffer]() {
      device->write(buffer->data());
      buffer->deleteLater();
      run(device);
    });
    return;
  }

  // If there are no more buffers - we are done!
  device->close();
  deleteLater();
}
