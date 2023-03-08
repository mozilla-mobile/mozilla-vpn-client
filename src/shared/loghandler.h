/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QDateTime>
#include <QMutexLocker>
#include <QObject>
#include <QVector>

#include "loglevel.h"

class QFile;
class QTextStream;

class LogHandler final : public QObject {
  Q_OBJECT

 public:
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

  static LogHandler* instance();

  static void messageQTHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& message);

  static void messageHandler(LogLevel logLevel, const QString& className,
                             const QString& message);

  static void rustMessageHandler(int32_t logLevel, char* message);

  static void prettyOutput(QTextStream& out, const LogHandler::Log& log);

  static void writeLogs(QTextStream& out);

  static void cleanupLogs();

  static void setLocation(const QString& path);

  static void enableStderr();

 signals:
  void logEntryAdded(const QByteArray& log);

 private:
  explicit LogHandler(const QMutexLocker<QMutex>& proofOfLock);

  static LogHandler* maybeCreate(const QMutexLocker<QMutex>& proofOfLock);

  void addLog(const Log& log, const QMutexLocker<QMutex>& proofOfLock);

  void openLogFile(const QMutexLocker<QMutex>& proofOfLock);

  void closeLogFile(const QMutexLocker<QMutex>& proofOfLock);

  static void cleanupLogFile(const QMutexLocker<QMutex>& proofOfLock);

  bool m_stderrEnabled = false;

  QFile* m_logFile = nullptr;
  QTextStream* m_output = nullptr;
};

#endif  // LOGHANDLER_H
