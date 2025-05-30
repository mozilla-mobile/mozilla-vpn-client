/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QDateTime>
#include <QList>
#include <QMutexLocker>
#include <QObject>
#include <QStandardPaths>

#ifdef MZ_IOS
#  include <os/log.h>
#endif

#include "loglevel.h"

class QBuffer;
class QFile;
class QTextStream;

class LogSerializer {
 public:
  // The name of this object to report in the encoded log file.
  virtual QString logName() const {
    return dynamic_cast<const QObject*>(this)->metaObject()->className();
  }

  // Output logs to the QIODevice and close when finished.
  virtual void logSerialize(QIODevice* output) = 0;
};

class LogHandler final : public QObject, public LogSerializer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LogHandler)

 public:
  LogHandler();
  static LogHandler* instance();

  struct Log {
    Log() = default;

    Log(LogLevel logLevel, const QString& className, const QString& message)
        : m_logLevel(logLevel),
          m_dateTime(QDateTime::currentDateTime()),
          m_className(className),
          m_message(message),
          m_fromQT(false) {}

    Log(LogLevel logLevel, const QString& file, const QString& function,
        uint32_t line, const QString& message)
        : m_logLevel(logLevel),
          m_dateTime(QDateTime::currentDateTime()),
          m_file(file),
          m_function(function),
          m_message(message),
          m_line(line),
          m_fromQT(true) {}

    LogLevel m_logLevel = LogLevel::Debug;
    QDateTime m_dateTime;
    QString m_file;
    QString m_function;
    QString m_className;
    QString m_message;
    int32_t m_line = -1;
    bool m_fromQT = false;
  };

  Q_INVOKABLE void retrieveLogs();
  Q_INVOKABLE void flushLogs();
  Q_INVOKABLE void requestViewLogs();

  static void messageQTHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& message);

  static void messageHandler(LogLevel logLevel, const QString& className,
                             const QString& message);

  static void rustMessageHandler(int32_t logLevel, char* message);

  static void prettyOutput(QTextStream& out, const LogHandler::Log& log);

  void writeLogs(QTextStream& out);

  void cleanupLogs();

  void setLocation(const QString& path);

  void setStderr(bool enabled = true);

  QString logFileName();

  QString logName() const override { return "MZ Logs"; }
  void logSerialize(QIODevice* device) override;

  bool writeLogsToLocation(
      QStandardPaths::StandardLocation location,
      std::function<void(const QString& filename)>&& a_callback);

  void registerLogSerializer(LogSerializer* logSerializer);
  void unregisterLogSerializer(LogSerializer* logSerializer);

 signals:
  void logEntryAdded(const QByteArray& log);
  void viewLogsNeeded();
  void logsReady(const QString& logs);
  void cleanupLogsNeeded();

 private:
  void addLog(const Log& log);
  void addLog(const Log& log, const QMutexLocker<QMutex>& proofOfLock);

  void openLogFile(const QMutexLocker<QMutex>& proofOfLock);

  void closeLogFile(const QMutexLocker<QMutex>& proofOfLock);

  void cleanupLogFile(const QMutexLocker<QMutex>& proofOfLock);

  void truncateLogFile(const QMutexLocker<QMutex>& proofOfLock,
                              const QString& filename);

  QMutex m_mutex;
  QString m_logShortName;

  bool m_stderrEnabled = false;

#ifdef MZ_IOS
  os_log_t m_ioslog;
#endif

  QFile* m_logFile = nullptr;
  QTextStream* m_output = nullptr;

  QList<LogSerializer*> m_logSerializers;
};

class LogSerializeHelper final : public QObject {
  Q_OBJECT

 public:
  LogSerializeHelper(QObject* parent = nullptr) : QObject(parent){};

  void addSerializer(LogSerializer* serializer);
  void run(QIODevice* device);

 private:
  int m_waiting;
  QList<QBuffer*> m_buffers;
};

#endif  // LOGHANDLER_H
