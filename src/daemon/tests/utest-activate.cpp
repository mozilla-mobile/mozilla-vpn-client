/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-activate.h"

#include <QString>
#include <QtTest/QtTest>

void TestActivate::activate() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));

  // Activate it.
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyConnected.isEmpty(), 5000);
  QVERIFY(spyDisconnected.isEmpty());
  QString connected = spyConnected.takeFirst().at(0).toString();
  QCOMPARE(connected, config.m_serverPublicKey);

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

void TestActivate::exclusionFailure() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Insert a conflicting LAN route before activation.
  IPAddress conflict = IPAddress("192.168.0.0/16");
  m_wgutils->setRoutePrefix(conflict, WireguardUtilsMock::RouteFailure);

  // Activate it.
  QVERIFY(!m_daemon->activate(config));

  // Check the activationFailure signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyFailure.isEmpty(), 5000);
  QVERIFY(spyConnected.isEmpty());
  QVERIFY(spyDisconnected.isEmpty());

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));

}

void TestActivate::routeFailure() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Insert a conflicting route before activation.
  IPAddress conflict = config.m_allowedIPAddressRanges.last();
  m_wgutils->setRoutePrefix(conflict, WireguardUtilsMock::RouteFailure);

  // Activate it.
  QVERIFY(!m_daemon->activate(config));

  // Check the activationFailure signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyFailure.isEmpty(), 5000);
  QVERIFY(spyConnected.isEmpty());
  QVERIFY(spyDisconnected.isEmpty());

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));
}

void TestActivate::dnsFailure() {
  InterfaceConfig config = exampleConfig();
  QSignalSpy spyConnected(m_daemon.get(), SIGNAL(connected(QString)));
  QSignalSpy spyDisconnected(m_daemon.get(), SIGNAL(disconnected()));
  QSignalSpy spyFailure(m_daemon.get(), SIGNAL(activationFailure()));

  // Flag the dnsutils to fail the next update.
  m_dnsutils->m_willFailUpdate = 1;

  // Activate it.
  QVERIFY(!m_daemon->activate(config));

  // Check the activationFailure signal.
  QTRY_VERIFY_WITH_TIMEOUT(!spyFailure.isEmpty(), 5000);
  QVERIFY(spyConnected.isEmpty());
  QVERIFY(spyDisconnected.isEmpty());

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));
}
