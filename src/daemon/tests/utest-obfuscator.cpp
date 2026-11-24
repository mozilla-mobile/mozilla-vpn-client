/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-obfuscator.h"

#include <QString>
#include <QtTest/QtTest>

#include "curve25519.h"

void TestObfuscator::activate() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  config.m_obfuscationMethod = Server::ObfuscationMethod::LWO;

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));

  // Activate it.
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QVERIFY(spyFailure.isEmpty());
  QString connected = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(connected, config.m_serverPublicKey);

  // Check that an obfuscator is running.
  QVERIFY(obfuscator() != nullptr);
  QVERIFY(obfuscator()->isRunning());

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

void TestObfuscator::switchServersAndMethod() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  config.m_obfuscationMethod = Server::ObfuscationMethod::LWO;

  // Activate it.
  QVERIFY(m_daemon->activate(config));
  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QVERIFY(spyFailure.isEmpty());
  QString connected = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(connected, config.m_serverPublicKey);

  // Check that an obfuscator is running.
  QVERIFY(obfuscator() != nullptr);
  QVERIFY(obfuscator()->isRunning());

  // Switch to a new server, with a different obfuscator method.
  InterfaceConfig next = exampleConfig('B');
  next.m_obfuscationMethod = Server::ObfuscationMethod::UdpOverTcp;
  QVERIFY(m_daemon->supportServerSwitching(next));
  QVERIFY(m_daemon->activate(next));

  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QVERIFY(spyFailure.isEmpty());
  connected = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(connected, next.m_serverPublicKey);

  // Check that an obfuscator is running.
  QVERIFY(obfuscator() != nullptr);
  QVERIFY(obfuscator()->isRunning());
}
