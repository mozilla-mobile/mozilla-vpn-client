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
#include "nulliodevice.h"

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

LogHandler::LogHandler() : QObject(nullptr), m_output(new NullIODevice()) {
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
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
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
  // If the log file has grown too big, truncate it before writing more logs.
  if (m_output.pos() > LOG_MAX_FILE_SIZE) {
    openLogFile(proofOfLock);
  }
  prettyOutput(m_output, log);

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
  m_output.flush();
  if (m_output.device()->isSequential()) {
    return;
  }
  if (m_output.seek(0)) {
    out << m_output.readAll();
  }
}

void LogHandler::cleanupLogs() {
  QMutexLocker<QMutex> lock(&m_mutex);
  cleanupLogFile(lock);
}

void LogHandler::cleanupLogFile(const QMutexLocker<QMutex>& proofOfLock) {
  Q_UNUSED(proofOfLock);
  m_output.flush();
  m_output.seek(0);

  QFileDevice* device = qobject_cast<QFileDevice*>(m_output.device());
  if (device) {
    device->resize(0);
  }
}

// static
void LogHandler::setLogfile(const QString& path) {
  if (!logHandler.exists()) {
    s_filename = path;
    return;
  }

  QMutexLocker<QMutex> lock(&logHandler->m_mutex);
  s_filename = path;
  logHandler->cleanupLogFile(lock);
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
  QFileDevice::Permissions perms =
      QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
      QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther |
      QFileDevice::ExeOther;
  return parent.mkdir(dir.dirName(), perms);
}

void LogHandler::setLogDevice(QIODevice* device,
                              const QMutexLocker<QMutex>& proofOfLock) {
  Q_UNUSED(proofOfLock);

  // Close the previous log device, if any.
  QIODevice* prevDevice = m_output.device();
  m_output.setDevice(device);
  if (prevDevice) {
    prevDevice->close();
    delete prevDevice;
  }

#ifdef MZ_DEBUG
  QString message;
  QFileDevice* filedev = qobject_cast<QFileDevice*>(device);
  if (filedev) {
    message = QString("Log file: %1").arg(filedev->fileName());
  } else {
    message = QString("Log device: %1").arg(device->metaObject()->className());
  }
  addLog(Log(Debug, "LogHandler", message), proofOfLock);
#endif
}

void LogHandler::openLogFile(const QMutexLocker<QMutex>& proofOfLock) {
  QDir appDataLocation = QFileInfo(s_filename).dir();
  if (!makeLogDir(appDataLocation)) {
    setLogDevice(new NullIODevice(), proofOfLock);
    return;
  }

  QFile* file = new QFile(s_filename);

  // If the log file is undersize, no truncation is needed.
  if (file->size() < LOG_MAX_FILE_SIZE) {
    if (!file->open(QIODevice::ReadWrite | QIODevice::Append |
                    QIODevice::Text)) {
      setLogDevice(new NullIODevice(), proofOfLock);
      delete file;
    } else {
      setLogDevice(file, proofOfLock);
    }
    return;
  }

  QString tempName = s_filename + ".bak";
  if (!file->rename(s_filename + ".bak")) {
    setLogDevice(file, proofOfLock);
    fprintf(stderr, "rename failed\n");
    return;
  }
  if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
    setLogDevice(new NullIODevice(), proofOfLock);
    return;
  }

  // Discard all but the tail of the log and align to the next new line.
  file->seek(file->size() - (LOG_MAX_FILE_SIZE / 2));
  file->readLine();
  auto guard = qScopeGuard([file] {
    file->remove();
    delete file;
  });

  // Re-create the original log file and copy the truncated contents.
  QFile* newLogFile = new QFile(s_filename);
  if (newLogFile->open(QIODevice::ReadWrite | QIODevice::Truncate |
                       QIODevice::Text)) {
    newLogFile->write(file->readAll());
    setLogDevice(newLogFile, proofOfLock);
  } else {
    // Failed to write the new log file - use a null device.
    setLogDevice(new NullIODevice(), proofOfLock);
    delete newLogFile;
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
    const QString& location,
    std::function<void(const QString& filename)>&& a_callback) {
#ifdef MZ_DEBUG
  logger.debug() << "Trying to save logs in:" << location;
#else
  logger.debug() << "Trying to save logs.";
#endif

  std::function<void(const QString& filename)> callback = std::move(a_callback);

  if (!QFileInfo::exists(location)) {
    return false;
  }

  QString filename;
  QDate now = QDate::currentDate();

  QTextStream(&filename) << m_shortname << "-" << now.year() << "-"
                         << now.month() << "-" << now.day() << LOG_FILE_SUFFIX;

  QDir logDir(location);
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
