/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtemporarydir.h"

#include "temporarydir.h"

void TestTemporaryDir::basic() {
  TemporaryDir td;
  QVERIFY(td.isValid());
  QVERIFY(td.errorString().isEmpty());
  QVERIFY(!td.path().isEmpty());
  QVERIFY(!td.filePath("wow").isEmpty());
}

void TestTemporaryDir::fallback() {
  QString path;
  {
    TemporaryDir td;
    td.fallback();

    QVERIFY(td.isValid());
    QVERIFY(td.errorString().isEmpty());
    path = td.path();
    QVERIFY(!path.isEmpty());
    QVERIFY(!td.filePath("wow").isEmpty());

    QVERIFY(QDir(path).exists());
  }

  QVERIFY(!path.isEmpty());
  QVERIFY(!QDir(path).exists());
}

void TestTemporaryDir::cleanupAll() {
  TemporaryDir td;
  td.fallback();

  QVERIFY(td.isValid());
  QString path = td.path();
  QVERIFY(QDir(path).exists());

  TemporaryDir::cleanupAll();
  QVERIFY(!QDir(path).exists());
  QVERIFY(!td.isValid());
}

QTEST_MAIN(TestTemporaryDir);
