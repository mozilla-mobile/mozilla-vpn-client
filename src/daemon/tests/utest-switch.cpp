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
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  m_config = exampleConfig();
  QVERIFY(m_daemon->activate(m_config));
  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);

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
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Prepare a switch to a new server.
  config.m_serverPublicKey = exampleKey("ServerPublicKey", 'B');
  config.m_serverIpv4AddrIn = "10.66.77.88";
  config.m_serverIpv6AddrIn = "fc02:dead::1234:5678";
  config.m_serverPort = 53;

  // Change routes too
  config.m_allowedIPAddressRanges.clear();
  config.m_allowedIPAddressRanges.append(IPAddress("0.0.0.0/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("172.16.0.0/16"));
  config.m_allowedIPAddressRanges.append(IPAddress("::/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("fc02:dead::/64"));

  QVERIFY(m_daemon->supportServerSwitching(config));
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QString connected = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(connected, config.m_serverPublicKey);

  // Check that we are connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(true));
  QVERIFY(status.contains("date"));

  // Verify the routing table.
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
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
    QVERIFY(config.m_allowedIPAddressRanges.contains(i.key()));
  }
}

// Full switching is required when the device configuration changes.
void TestSwitch::hardSwitch() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Change the device keys.
  config.m_privateKey = exampleKey("TestPrivateKey", 'B');
  QByteArray key = config.m_privateKey.toUtf8();
  config.m_publicKey = QString::fromUtf8(Curve25519::generatePublicKey(key));

  // Prepare a switch to a new server.
  config.m_serverPublicKey = exampleKey("ServerPublicKey", 'B');
  config.m_serverIpv4AddrIn = "10.66.77.88";
  config.m_serverIpv6AddrIn = "fc02:dead::1234:5678";
  config.m_serverPort = 53;

  // Change routes too
  config.m_allowedIPAddressRanges.clear();
  config.m_allowedIPAddressRanges.append(IPAddress("0.0.0.0/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("172.16.0.0/16"));
  config.m_allowedIPAddressRanges.append(IPAddress("::/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("fc02:dead::/64"));

  QVERIFY(!m_daemon->supportServerSwitching(config));
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QString connected = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(connected, config.m_serverPublicKey);

  // Check that we are connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(true));
  QVERIFY(status.contains("date"));

  // Verify the routing table.
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
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
    QVERIFY(config.m_allowedIPAddressRanges.contains(i.key()));
  }
}
