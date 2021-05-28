/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QDateTime>
#include <QObject>
#include <QVector>

class QFile;
class QMutexLocker;
class QTextStream;

class LogHandler final : public QObject {
  Q_OBJECT

 public:
  struct Log {
    Log() = default;

    Log(const QStringList& modules, const QString& className,
        const QString& message)
        : m_dateTime(QDateTime::currentDateTime()),
          m_modules(modules),
          m_className(className),
          m_message(message),
          m_fromQT(false) {}

    Log(QtMsgType type, const QString& file, const QString& function,
        uint32_t line, const QString& message)
        : m_dateTime(QDateTime::currentDateTime()),
          m_file(file),
          m_function(function),
          m_message(message),
          m_type(type),
          m_line(line),
          m_fromQT(true) {}

    QDateTime m_dateTime;
    QString m_file;
    QString m_function;
    QStringList m_modules;
    QString m_className;
    QString m_message;
    QtMsgType m_type = QtMsgType::QtDebugMsg;
    int32_t m_line = -1;
    bool m_fromQT = false;
  };

  static LogHandler* instance();

  static void messageQTHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& message);

  static void messageHandler(const QStringList& modules,
                             const QString& className, const QString& message);

  static void prettyOutput(QTextStream& out, const LogHandler::Log& log);

  static void writeLogs(QTextStream& out);

  static void cleanupLogs();

  static void setLocation(const QString& path);

 signals:
  void logEntryAdded(const QByteArray& log);

 private:
  LogHandler(const QStringList& modules, const QMutexLocker& proofOfLock);

  static LogHandler* maybeCreate(const QMutexLocker& proofOfLock);

  void addLog(const Log& log, const QMutexLocker& proofOfLock);

  bool matchModule(const Log& log, const QMutexLocker& proofOfLock) const;

  void openLogFile(const QMutexLocker& proofOfLock);

  void closeLogFile(const QMutexLocker& proofOfLock);

  static void cleanupLogFile(const QMutexLocker& proofOfLock);

  const QStringList m_modules;

  QFile* m_logFile = nullptr;
  QTextStream* m_output = nullptr;
};

#endif  // LOGHANDLER_H
