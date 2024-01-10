/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlogger.h"

#include <QScopeGuard>

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
  LogHandler::setStderr(false);
  auto guard = qScopeGuard([&] { LogHandler::setStderr(true); });

  // Write a megabyte log data.
  const QString example = "All work and no play makes Jack a dull boy";
  qsizetype count = (1024 * 1024) / example.size();
  while (count-- > 0) {
    l.info() << example;
  }

  // Write the logs, and we should get well in excess of 1MB of text.
  {
    QString hugeBuffer;
    QTextStream out(&hugeBuffer);
    lh->writeLogs(out);
    QVERIFY(hugeBuffer.size() > 1024 * 1024);
  }

  // After writing the logs, the log file should still contain roughly
  // LOG_MAX_FILE_SIZE/2 bytes of log data.
  QString truncatedBuffer;
  QTextStream out(&truncatedBuffer);
  lh->writeLogs(out);
  QVERIFY(truncatedBuffer.size() > 64 * 1024);
  QVERIFY(truncatedBuffer.size() < 128 * 1024);
}

QTEST_MAIN(TestLogger);
