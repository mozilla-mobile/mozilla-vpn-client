/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include "loglevel.h"

#include <QDateTime>
#include <QObject>
#include <QVector>
#include <mutex>

class QFile;
class QTextStream;

class LogHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LogHandler)
 public:
  struct Log {
    Log() = default;

    Log(LogLevel logLevel, const QStringList& modules, const QString& className,
        const QString& message)
        : m_logLevel(logLevel),
          m_dateTime(QDateTime::currentDateTime()),
          m_modules(modules),
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
    QStringList m_modules;
    QString m_className;
    QString m_message;
    int32_t m_line = -1;
    bool m_fromQT = false;
  };

  static LogHandler& instance();

  static void messageQTHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& message);

  static void messageHandler(LogLevel logLevel, const QStringList& modules,
                             const QString& className, const QString& message);

  static void prettyOutput(QTextStream& out, const LogHandler::Log& log);

  static void writeLogs(QTextStream& out);

  static void cleanupLogs();

  static void setLocation(const QString& path);

  static void enableDebug();

 signals:
  void logEntryAdded(const QByteArray& log);

 private:
  LogHandler();

  static LogHandler* maybeCreate();

  void addLog(const Log& log);

  bool matchLogLevel(const Log& log) const;
  bool matchModule(const Log& log) const;

  void openLogFile();

  void closeLogFile();

  static void cleanupLogFile();

  LogLevel m_minLogLevel;
  QStringList m_modules;
  bool m_showDebug = false;

  QFile* m_logFile = nullptr;
  QTextStream* m_output = nullptr;

  std::recursive_mutex m_mutex;
};

#endif  // LOGHANDLER_H
