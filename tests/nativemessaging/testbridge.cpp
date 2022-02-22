/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testbridge.h"
#include "helperserver.h"

void TestBridge::bridge_ping() {
  QVERIFY(s_nativeMessagingProcess);

  // A simple ping/pong.
  QVERIFY(write("\"bridge_ping\""));
  QCOMPARE(readIgnoringStatus(), "\"bridge_pong\"");
}

void TestBridge::app_ping_failure() {
  QVERIFY(s_nativeMessagingProcess);

  // No VPN client running, we want to receive a "down" status for each
  // message.
  for (int i = 0; i < 3; ++i) {
    QVERIFY(write("\"ping\""));
    QCOMPARE(readIgnoringStatus(), "{\"error\":\"vpn-client-down\"}");
  }
}

void TestBridge::app_ping_success() {
  QVERIFY(s_nativeMessagingProcess);

  HelperServer hs;
  hs.start();

  // Let's turn on a "VPN client" (echo-server)...
  QEventLoop loop;
  connect(&hs, &HelperServer::ready, [&] { loop.exit(); });
  loop.exec();

  // let's wait for a client-up message
  QVERIFY(waitForConnection());

  // Now we want to receive our messages back.
  for (int i = 0; i < 3; ++i) {
    QVERIFY(write("\"hello world\""));
    QCOMPARE(read(), "\"hello world\"");
  }

  hs.stop();
}

void TestBridge::async_connection() {
  QVERIFY(s_nativeMessagingProcess);

  bool started = false;
  HelperServer hs;

  while (true) {
    QVERIFY(write("\"async connection\""));

    if (!started) {
      QCOMPARE(readIgnoringStatus(), "{\"error\":\"vpn-client-down\"}");

      hs.start();
      started = true;

      QVERIFY(waitForConnection());
      continue;
    }

    QCOMPARE(read(), "\"async connection\"");
    break;
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

  QVERIFY(waitForConnection());

  bool stopped = false;

  while (true) {
    QVERIFY(write("\"async disconnection\""));

    QByteArray body = readIgnoringStatus();
    if (!stopped) {
      QCOMPARE(body, "\"async disconnection\"");

      hs.stop();
      stopped = true;
      continue;
    }

    QCOMPARE(body, "{\"error\":\"vpn-client-down\"}");
    break;
  }
}

static TestBridge s_testBridge;
