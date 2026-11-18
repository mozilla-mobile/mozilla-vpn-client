/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QObject>
#include <QtTest/QtTest>

#include "testdaemon.h"

int main(int argc, char* argv[]) {
  QCoreApplication::setApplicationName("Mozilla VPN Daemon Tests");
  QCoreApplication::setOrganizationName("Mozilla Testing");
  QCoreApplication app(argc, argv);

  // Execute the tests
  int numFails = 0;
  int numTests = 0;
  for (const QMetaObject* meta : TestDaemon::testcases()) {
    QObject* obj = reinterpret_cast<QObject*>(meta->metaType().create());

    numTests++;
    if (QTest::qExec(obj) != 0) {
      numFails++;
    }
    delete obj;
  }
  return numFails;
}
