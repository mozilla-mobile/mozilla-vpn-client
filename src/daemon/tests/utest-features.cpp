/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-features.h"

#include <QString>
#include <QtTest/QtTest>

void TestFeatures::features() {
  // Send the status request.
  QJsonObject obj;
  obj.insert("type", "status");
  send(obj);

  QCoreApplication::setApplicationVersion("1.2.66");

  QJsonObject reply;
  QVERIFY(recv(reply));
  QVERIFY(reply.contains("version"));
  QCOMPARE(reply.value("version").toString(), "1.2.66");
  QVERIFY(reply.contains("features"));
}
