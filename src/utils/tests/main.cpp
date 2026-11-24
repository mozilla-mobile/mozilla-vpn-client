/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QObject>
#include <QtTest/QtTest>

#include "leakdetector.h"
#include "loghandler.h"
#include "testhelper.h"

int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif
  QCoreApplication::setApplicationName("Mozilla VPN Unit Tests");
  QCoreApplication::setOrganizationName("Mozilla Testing");
  LogHandler::instance()->setStderr(true);
  QCoreApplication app(argc, argv);

  // Execute the tests
  int numFails = 0;
  for (const QMetaObject* meta : TestHelper::testcases()) {
    QObject* obj = reinterpret_cast<QObject*>(meta->metaType().create());
    if (QTest::qExec(obj) != 0) {
      numFails++;
    }
    delete obj;
  }
  return numFails;
}
