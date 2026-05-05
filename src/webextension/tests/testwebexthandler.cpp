/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testwebexthandler.h"

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "webexthandler.h"

QTEST_MAIN(TestWebExtHandler)

// Read a single length-prefixed native-messaging message from the buffer.
static QByteArray readMsg(QBuffer& buf) {
  buf.seek(0);
  quint32 len = 0;
  if (buf.read(reinterpret_cast<char*>(&len), sizeof(len)) !=
      qint64(sizeof(len))) {
    return {};
  }
  return buf.read(len);
}

void TestWebExtHandler::bridge_ping() {
  QBuffer output;
  output.open(QIODevice::ReadWrite);
  WebExtHandler handler(&output, false);

  handler.bridge_ping(QByteArray(R"({"t":"bridge_ping"})"));

  auto doc = QJsonDocument::fromJson(readMsg(output));
  QCOMPARE(doc["status"].toString(), "bridge_pong");
}

void TestWebExtHandler::proc_info() {
  QBuffer output;
  output.open(QIODevice::ReadWrite);
  WebExtHandler handler(&output, false);

  handler.proc_info(QByteArray(R"({"t":"proc_info"})"));

  auto doc = QJsonDocument::fromJson(readMsg(output));
  QVERIFY(doc["pid"].toInteger() > 0);
  QVERIFY(!doc["exe"].toString().isEmpty());
}
