/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testbridge.h"
#include "helperserver.h"

void TestBridge::bridge_ping() {
  QVERIFY(s_nativeMessagingProcess);

  QVERIFY(write("\"bridge_ping\""));
  QCOMPARE(read(), "\"bridge_pong\"");
}

void TestBridge::app_ping_failure() {
  QVERIFY(s_nativeMessagingProcess);

  QVERIFY(write("\"ping\""));
  QCOMPARE(read(), "{\"error\":\"vpn-client-down\"}");
}

void TestBridge::app_ping_success() {
  QVERIFY(s_nativeMessagingProcess);

  HelperServer hs;
  hs.start();

  QEventLoop loop;
  connect(&hs, &HelperServer::ready, [&] { loop.exit(); });
  loop.exec();

  QVERIFY(write("\"hello world\""));
  QCOMPARE(read(), "\"hello world\"");

  hs.stop();
}

void TestBridge::async_connection() {
  QVERIFY(s_nativeMessagingProcess);

  bool started = false;
  HelperServer hs;

  while (true) {
    QVERIFY(write("\"hello world\""));

    if (!started) {
      hs.start();
      started = true;
    }

    QByteArray body = read();
    if (body == "\"hello world\"") {
      break;
    }

    QCOMPARE(body, "{\"error\":\"vpn-client-down\"}");
  }

  hs.stop();
}

void TestBridge::async_disconnection() {
  QVERIFY(s_nativeMessagingProcess);

  HelperServer hs;
  hs.start();

  QEventLoop loop;
  connect(&hs, &HelperServer::ready, [&] { loop.exit(); });
  loop.exec();

  bool stopped = false;

  while (true) {
    QVERIFY(write("\"async disconnection\""));

    if (!stopped) {
      hs.stop();
      stopped = true;
    }

    QByteArray body = tryToRead();
    if (body == "\"async disconnection\"" || body == "") {
      continue;
    }

    QCOMPARE(body, "{\"error\":\"vpn-client-down\"}");
    break;
  }
}

static TestBridge s_testBridge;
