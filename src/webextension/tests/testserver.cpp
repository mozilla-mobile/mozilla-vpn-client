/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testserver.h"

#include <QHostAddress>
#include <QTest>

#include "server.h"

QTEST_MAIN(TestServer)

struct TestCase {
  QHostAddress addr;
  bool excepted;
};

void TestServer::testCanConnect() {
  QList<TestCase> cases{
      TestCase{QHostAddress("::ffff:127.0.0.1"), true},
      TestCase{QHostAddress::LocalHost, true},
      TestCase{QHostAddress::LocalHostIPv6, true},
      TestCase{QHostAddress("0.0.0.0"), false},
      TestCase{QHostAddress("1.2.3.4"), false},
  };
  for (auto& c : cases) {
    QCOMPARE(WebExtension::Server::isAllowedToConnect(c.addr), c.excepted);
  }
}
