/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

#include <QCoreApplication>

QVector<QObject*> TestHelper::testList;

TestHelper::TestHelper() { testList.append(this); }

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);

  int failures = 0;
  for (QObject* obj : TestHelper::testList) {
    int result = QTest::qExec(obj);
    if (result != 0) {
      ++failures;
    }
  }

  return failures;
}
