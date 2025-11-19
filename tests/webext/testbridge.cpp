/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testbridge.h"

#include <QJsonDocument>

#include "helperserver.h"

void TestBridge::bridge_ping() {
  // A simple ping/pong.
  QVERIFY(write(R"({"t": "bridge_ping"})"));
  auto const json = QJsonDocument::fromJson(readIgnoringStatus());
  QCOMPARE(json["status"].toString(), "bridge_pong");
}

void TestBridge::proc_info() {
  qint64 selfpid = QCoreApplication::applicationPid();
  QString selfexe = QCoreApplication::applicationFilePath();

  // The proc_info command should get the parent process details, which
  // should be this test binary.
  QVERIFY(write(R"({"t": "proc_info"})"));
  auto const json = QJsonDocument::fromJson(readIgnoringStatus());
  QCOMPARE(json["pid"].toInteger(), selfpid);
  QCOMPARE(json["exe"].toString(), QDir::toNativeSeparators(selfexe));
}

void TestBridge::app_ping_failure() {
  // No VPN client running, we want to receive a "down" status for each
  // message.
  for (int i = 0; i < 3; ++i) {
    QVERIFY(write("\"ping\""));
    QCOMPARE(readIgnoringStatus(), "{\"error\":\"vpn-client-down\"}");
  }
}

void TestBridge::app_ping_success() {
  HelperServer hs;
  hs.start(serverName());

  // Let's turn on a "VPN client" (echo-server)...
  QEventLoop loop;
  connect(&hs, &HelperServer::ready, &hs, [&] { loop.exit(); });
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
  bool started = false;
  HelperServer hs;

  while (true) {
    QVERIFY(write("\"async connection\""));

    if (!started) {
      QCOMPARE(readIgnoringStatus(), "{\"error\":\"vpn-client-down\"}");

      hs.start(serverName());
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
  HelperServer hs;
  hs.start(serverName());

  QEventLoop loop;
  connect(&hs, &HelperServer::ready, &hs, [&] { loop.exit(); });
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

void TestBridge::fuzzy() {
  for (int fuzzy : QList<int>{1, 10, 100}) {
    HelperServer hs;
    hs.start(serverName(), fuzzy);

    // Let's turn on a "VPN client" (echo-server)...
    QEventLoop loop;
    connect(&hs, &HelperServer::ready, &hs, [&] { loop.exit(); });
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
}

static TestBridge s_testBridge;
