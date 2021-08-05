/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlogger.h"
#include "../../src/logger.h"
#include "../../src/loghandler.h"
#include "helper.h"

void TestLogger::logger() {
  Logger l("test", "class");
  l.info() << "Hello world" << 42 << 'a' << QString("OK") << QByteArray("Array")
          << QStringList{"A", "B"} << Qt::endl;

  Logger l2(QStringList{"a", "b"}, "class");
  l2.info() << "Hello world" << 42 << 'a' << QString("OK") << QByteArray("Array")
           << QStringList{"A", "B"} << Qt::endl;
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

static TestLogger s_testLogger;
