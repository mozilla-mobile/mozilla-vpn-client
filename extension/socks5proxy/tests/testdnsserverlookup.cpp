/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <qhostaddress.h>

#include <QObject>
#include <QTest>

#include "dnsserverlookup.h"

class TestDNSServerLookup final : public QObject {
  Q_OBJECT

 private slots:
  void returnsSomeDNS();
};

QTEST_MAIN(TestDNSServerLookup)

void TestDNSServerLookup::returnsSomeDNS() {
  /**
   * Well given this checks your system config,
   * we can't assert anything but that this should return __some__
   * QHostaddress
   */
  auto const result = DNSServerLookup::getLocalDNSName();
  QVERIFY(result.has_value());
  QHostAddress dns = result.value();
  QVERIFY(!dns.isNull());
  QVERIFY(!dns.isLoopback());
}

#include "testdnsserverlookup.moc"
