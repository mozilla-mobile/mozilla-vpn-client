/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-switch.h"

#include <QString>
#include <QtTest/QtTest>

#include "curve25519.h"

void TestSwitch::init() {
  // Setup the daemon testcase.
  TestDaemon::init();

  // Setup the initial connection.
  m_config = exampleConfig();
  QVERIFY(m_daemon->activate(m_config));
  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), m_config.m_serverPublicKey);
  QCOMPARE(m_disconnectedEventCount, 0);
  QVERIFY(m_failureEvents.isEmpty());

  // Check that we are connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(true));
  QVERIFY(status.contains("date"));

  // Verify the routing table.
  for (const IPAddress& ip : m_config.m_allowedIPAddressRanges) {
    const QString msg("m_wgutils->routeTable().contains() failed for %1");
    QVERIFY2(m_wgutils->routes().contains(ip),
             qPrintable(msg.arg(ip.toString())));
    QCOMPARE(m_wgutils->routes().value(ip), WireguardUtilsMock::RoutePeer);
  }
}

// Switch to a new peer.
void TestSwitch::basicSwitch() {
  InterfaceConfig next = exampleConfig();

  // Prepare a switch to a new server.
  next.m_serverPublicKey = exampleKey("ServerPublicKey", 'B');
  next.m_serverIpv4AddrIn = "10.66.77.88";
  next.m_serverIpv6AddrIn = "fc02:dead::1234:5678";
  next.m_serverPort = 53;

  // Change routes too
  next.m_allowedIPAddressRanges.clear();
  next.m_allowedIPAddressRanges.append(IPAddress("0.0.0.0/0"));
  next.m_allowedIPAddressRanges.append(IPAddress("172.16.0.0/16"));
  next.m_allowedIPAddressRanges.append(IPAddress("::/0"));
  next.m_allowedIPAddressRanges.append(IPAddress("fc02:dead::/64"));

  QVERIFY(m_daemon->supportServerSwitching(next));
  QVERIFY(m_daemon->activate(next));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), next.m_serverPublicKey);
  QCOMPARE(m_disconnectedEventCount, 0);
  QVERIFY(m_failureEvents.isEmpty());

  // Check that we are connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(true));
  QVERIFY(status.contains("date"));

  // Verify the routing table.
  for (const IPAddress& ip : next.m_allowedIPAddressRanges) {
    const QString msg("m_wgutils->routeTable().contains() failed for %1");
    QVERIFY2(m_wgutils->routes().contains(ip),
             qPrintable(msg.arg(ip.toString())));
    QCOMPARE(m_wgutils->routes().value(ip), WireguardUtilsMock::RoutePeer);
  }
  // Check for stale routes that were owned by the old peer.
  for (auto i = m_wgutils->routes().begin(); i != m_wgutils->routes().end();
       i++) {
    if (i.value() != WireguardUtilsMock::RoutePeer) {
      continue;
    }
    QVERIFY(next.m_allowedIPAddressRanges.contains(i.key()));
  }
}

// Full switching is required when the device configuration changes.
void TestSwitch::hardSwitch() {
  InterfaceConfig next = exampleConfig();

  // Change the device keys.
  next.m_privateKey = exampleKey("TestPrivateKey", 'B');
  QByteArray key = next.m_privateKey.toUtf8();
  next.m_publicKey = QString::fromUtf8(Curve25519::generatePublicKey(key));

  // Prepare a switch to a new server.
  next.m_serverPublicKey = exampleKey("ServerPublicKey", 'B');
  next.m_serverIpv4AddrIn = "10.66.77.88";
  next.m_serverIpv6AddrIn = "fc02:dead::1234:5678";
  next.m_serverPort = 53;

  // Change routes too
  next.m_allowedIPAddressRanges.clear();
  next.m_allowedIPAddressRanges.append(IPAddress("0.0.0.0/0"));
  next.m_allowedIPAddressRanges.append(IPAddress("172.16.0.0/16"));
  next.m_allowedIPAddressRanges.append(IPAddress("::/0"));
  next.m_allowedIPAddressRanges.append(IPAddress("fc02:dead::/64"));

  QVERIFY(!m_daemon->supportServerSwitching(next));
  QVERIFY(m_daemon->activate(next));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), next.m_serverPublicKey);
  QCOMPARE(m_disconnectedEventCount, 0);
  QVERIFY(m_failureEvents.isEmpty());

  // Check that we are connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(true));
  QVERIFY(status.contains("date"));

  // Verify the routing table.
  for (const IPAddress& ip : next.m_allowedIPAddressRanges) {
    const QString msg("m_wgutils->routeTable().contains() failed for %1");
    QVERIFY2(m_wgutils->routes().contains(ip),
             qPrintable(msg.arg(ip.toString())));
    QCOMPARE(m_wgutils->routes().value(ip), WireguardUtilsMock::RoutePeer);
  }

  // Check for stale routes that were owned by the old peer.
  for (auto i = m_wgutils->routes().begin(); i != m_wgutils->routes().end();
       i++) {
    if (i.value() != WireguardUtilsMock::RoutePeer) {
      continue;
    }
    QVERIFY(next.m_allowedIPAddressRanges.contains(i.key()));
  }
}
