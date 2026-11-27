/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-activate.h"

#include <QString>
#include <QtTest/QtTest>

void TestActivate::activate() {
  InterfaceConfig config = exampleConfig();

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));

  // Activate it.
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_disconnectedEventCount, 0);
  QVERIFY(m_failureEvents.isEmpty());
  QCOMPARE(m_connectedEvents.takeFirst(), config.m_serverPublicKey);

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

  // Insert a conflicting LAN route before activation.
  IPAddress conflict = IPAddress("192.168.0.0/16");
  m_wgutils->setRoutePrefix(conflict, WireguardUtilsMock::RouteFailure);

  // Activate it.
  QVERIFY(!m_daemon->activate(config));

  // Check the backendFailure signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_failureEvents.isEmpty(), 5000);
  QCOMPARE(m_failureEvents.takeFirst(), DaemonError::ERROR_ACTIVATION_FAILURE);
  QVERIFY(m_connectedEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));
}

void TestActivate::routeFailure() {
  InterfaceConfig config = exampleConfig();

  // Insert a conflicting route before activation.
  IPAddress conflict = config.m_allowedIPAddressRanges.last();
  m_wgutils->setRoutePrefix(conflict, WireguardUtilsMock::RouteFailure);

  // Activate it.
  QVERIFY(!m_daemon->activate(config));

  // Check the activationFailure signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_failureEvents.isEmpty(), 5000);
  QCOMPARE(m_failureEvents.takeFirst(), DaemonError::ERROR_ACTIVATION_FAILURE);
  QVERIFY(m_connectedEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));
}

void TestActivate::dnsFailure() {
  InterfaceConfig config = exampleConfig();

  // Flag the dnsutils to fail the next update.
  m_dnsutils->m_willFailUpdate = 1;

  // Activate it.
  QVERIFY(!m_daemon->activate(config));

  // Check the activationFailure signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_failureEvents.isEmpty(), 5000);
  QCOMPARE(m_failureEvents.takeFirst(), DaemonError::ERROR_ACTIVATION_FAILURE);
  QVERIFY(m_connectedEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));
}
