/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-multihop.h"

#include <QString>
#include <QtTest/QtTest>

void TestMultihop::activate() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));

  // Configure multihop
  config.m_multihopPublicKey = exampleKey("MultihopEntryKey", 'B');
  config.m_multihopCityName = "Rivendell";
  config.m_multihopIpv4AddrIn = "10.123.88.99";
  config.m_multihopIpv6AddrIn = "fc02:abcd::dead:beef";
  config.m_multihopPort = 1234;

  // Activate it.
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(spyConnected.length() == 2, 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QString entryPubkey = spyConnected.takeFirst().at(0).toString();
  QString exitPubkey = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(entryPubkey, config.m_multihopPublicKey);
  QCOMPARE(exitPubkey, config.m_serverPublicKey);

  // Check that we are connected.
  status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(true));
  QVERIFY(status.contains("date"));

  // Verify the routing table.
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    const QString msg("m_wgutils->routeTable().contains() failed for %1");
    QVERIFY2(m_wgutils->routes().contains(ip),
             qPrintable(msg.arg(ip.toString())));
    QCOMPARE(m_wgutils->routes().value(ip), WireguardUtilsMock::RoutePeer);
  }
}
