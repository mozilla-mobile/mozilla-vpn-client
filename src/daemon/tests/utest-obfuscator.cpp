/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utest-obfuscator.h"

#include <QString>
#include <QtTest/QtTest>

#include <sys/types.h>
#include <signal.h>

#include "curve25519.h"

void TestObfuscator::activate() {
  InterfaceConfig config = exampleConfig();
  config.m_obfuscationMethod = Server::ObfuscationMethod::LWO;

  // Check that we are not connected.
  QJsonObject status = m_daemon->getStatus();
  QCOMPARE(status.value("connected"), QJsonValue(false));
  QVERIFY(!status.contains("date"));

  // Activate it.
  QVERIFY(m_daemon->activate(config));

  // Check the connected signal.
  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), config.m_serverPublicKey);
  QVERIFY(m_failureEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

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
  // Start with the activation test
  activate();

  // Switch to a new server, with a different obfuscator method.
  InterfaceConfig next = exampleConfig('B');
  next.m_obfuscationMethod = Server::ObfuscationMethod::UdpOverTcp;
  QVERIFY(m_daemon->supportServerSwitching(next));
  QVERIFY(m_daemon->activate(next));

  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), next.m_serverPublicKey);
  QVERIFY(m_failureEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

  // Check that an obfuscator is running.
  QVERIFY(obfuscator() != nullptr);
  QVERIFY(obfuscator()->isRunning());
}

void TestObfuscator::switchServerAddObfuscator() {
  InterfaceConfig config = exampleConfig();

  // Activate it.
  QVERIFY(m_daemon->activate(config));
  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), config.m_serverPublicKey);
  QVERIFY(m_failureEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

  // Check that no obfuscator is running.
  QCOMPARE(obfuscator(), nullptr);

  // Switch to a new server, with an added obfuscator method.
  InterfaceConfig next = exampleConfig('B');
  next.m_obfuscationMethod = Server::ObfuscationMethod::LWO;
  QVERIFY(m_daemon->supportServerSwitching(next));
  QVERIFY(m_daemon->activate(next));

  QTRY_VERIFY_WITH_TIMEOUT(!m_connectedEvents.isEmpty(), 5000);
  QCOMPARE(m_connectedEvents.takeFirst(), next.m_serverPublicKey);
  QVERIFY(m_failureEvents.isEmpty());
  QCOMPARE(m_disconnectedEventCount, 0);

  // Check that an obfuscator is running.
  QVERIFY(obfuscator() != nullptr);
  QVERIFY(obfuscator()->isRunning());
}

void TestObfuscator::crash() {
  // Start with the activation test
  activate();
  QVERIFY(m_failureEvents.isEmpty());

  // Kill the obfuscator with a SIGSEGV
  kill((pid_t)obfuscator()->processId(), SIGSEGV);
  QTRY_VERIFY_WITH_TIMEOUT(!m_failureEvents.isEmpty(), 5000);
  QCOMPARE(m_failureEvents.takeFirst(), DaemonError::ERROR_OBFUSCATOR_CRASHED);
}
