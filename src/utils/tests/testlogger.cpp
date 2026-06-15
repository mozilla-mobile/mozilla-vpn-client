/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlogger.h"

#include <QScopeGuard>
#include <QtTest/QtTest>

#include "logger.h"
#include "loghandler.h"

void TestLogger::logger() {
  Logger l("class");
  l.info() << "Hello world" << 42 << 'a' << QString("OK") << QByteArray("Array")
           << QStringList{"A", "B"} << Qt::endl;

  Logger l2("class");
  l2.info() << "Hello world" << 42 << 'a' << QString("OK")
            << QByteArray("Array") << QStringList{"A", "B"} << Qt::endl;
}

void TestLogger::logHandler() {
  LogHandler* lh = LogHandler::instance();
  qInstallMessageHandler(LogHandler::messageQTHandler);

  qDebug() << "WOW debug!";
  qInfo() << "WOW info!";
  qWarning() << "WOW warning!";
  qCritical() << "WOW critical!";

  {
    QString buffer;
    QTextStream out(&buffer);
    lh->writeLogs(out);
  }

  lh->cleanupLogs();

  {
    QString buffer;
    QTextStream out(&buffer);
    lh->writeLogs(out);
  }
}

void TestLogger::logTruncation() {
  LogHandler* lh = LogHandler::instance();
  Logger l("test");

  // Disable stderr for this test, it's going to be a *lot*
  // turn it back on when we're done too.
  LogHandler::instance()->setStderr(false);
  auto guard = qScopeGuard([&] { LogHandler::instance()->setStderr(true); });

  // Keep track of how much data was written.
  qsizetype total = 0;
  QObject::connect(
      LogHandler::instance(), &LogHandler::logEntryAdded, this,
      [&total](const QByteArray& msg, LogLevel level) { total += msg.size(); });

  // Log truncation is somewhat inexact, and can vary by a line or two in either
  // direction.
  constexpr const int EPSILON = 1024;
  constexpr const int MEGABYTE = 1024 * 1024;

  // Write a megabyte log data.
  const QString example = "All work and no play makes Jack a dull boy";
  qsizetype count = MEGABYTE / example.size();
  while (count-- > 0) {
    l.debug() << example;

    // At no point should the logs ever exceed LOG_MAX_FILE_SIZE plus one line.
    QFileInfo info(LogHandler::s_filename);
    QVERIFY(info.size() < LogHandler::LOG_MAX_FILE_SIZE + EPSILON);
  }
  // There should be well over 1MB of text written so far.
  QVERIFY(total > MEGABYTE);

  // Write some more messages until we get close to truncation.
  while (true) {
    QFileInfo info(LogHandler::s_filename);
    if (info.size() >= LogHandler::LOG_MAX_FILE_SIZE) {
      break;
    }
    l.debug() << example;
  }

  // The next line written should truncate the logs.
  l.warning() << "REDRUM";

  // Check that the file on disk has been truncated.
  qsizetype logFileSize = QFileInfo(LogHandler::s_filename).size();
  QVERIFY(logFileSize > (LogHandler::LOG_MAX_FILE_SIZE / 2) - EPSILON);
  QVERIFY(logFileSize < (LogHandler::LOG_MAX_FILE_SIZE / 2) + EPSILON);

  // Write the logs, and we should get approximately half the max log size.
  QByteArray logBuffer;
  QTextStream out(&logBuffer);
  lh->writeLogs(out);

  // Due to line-ending conversion - we may need a little more wiggle room
  // on Windows since the length will strink as carriage-returns get stripped.
#ifdef Q_OS_WIN
  const int NEWLINE_SHRINKAGE = logBuffer.size() / example.size();
#else
  const int NEWLINE_SHRINKAGE = 0;
#endif

  QVERIFY(logBuffer.size() >
          (LogHandler::LOG_MAX_FILE_SIZE / 2) - EPSILON - NEWLINE_SHRINKAGE);
  QVERIFY(logBuffer.size() < (LogHandler::LOG_MAX_FILE_SIZE / 2) + EPSILON);
  QVERIFY(logBuffer.last(EPSILON).contains("REDRUM"));
}
