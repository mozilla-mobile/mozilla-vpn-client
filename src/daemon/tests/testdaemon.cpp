/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdaemon.h"

#include <QDir>
#include <QLocalSocket>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>
#include <QtTest/QtTest>

#include "curve25519.h"
#include "mock/dnsutilsmock.h"
#include "mock/mockdaemon.h"
#include "mock/wireguardutilsmock.h"

static QString randomSocketPath() {
  QString tail = QString::number(QRandomGenerator::global()->generate64(), 16);
#if defined(Q_OS_WIN)
  return QString("\\\\.\\pipe\\mozillavpn-mock-%1").arg(tail);
#else
  return QString("/var/tmp/mozillavpn-mock-%1.sock").arg(tail);
#endif
}

void TestDaemon::init() {
  QString socketPath = randomSocketPath();

  // Start the mock daemon.
  m_daemon.reset(new MockDaemon(socketPath, this));
#ifndef Q_OS_WIN
  QVERIFY(QFileInfo::exists(socketPath));
#endif
  m_dnsutils = qobject_cast<DnsUtilsMock*>(m_daemon->dnsutils());
  QVERIFY(m_dnsutils);
  m_wgutils = qobject_cast<WireguardUtilsMock*>(m_daemon->wgutils());
  QVERIFY(m_wgutils);

  // Utilize shorter timeouts for unit testing.
  m_daemon->m_handshakeTimer.setInterval(10);
  m_wgutils->setHandshakeDelay(50);

  // Connect to the mock daemon.
  m_socket.reset(new QLocalSocket(this));
  m_socket->connectToServer(socketPath);
  QVERIFY(m_socket->waitForConnected());
}

void TestDaemon::cleanup() {
  // If the daemon is connected, gracefully disconnect it.
  if (m_wgutils->interfaceExists()) {
    // Deactivate it.
    QSignalSpy spy(m_daemon.get(), SIGNAL(disconnected()));
    QVERIFY(m_daemon->deactivate());
    QTRY_VERIFY_WITH_TIMEOUT(!spy.isEmpty(), 5000);

    // Check that we are not connected.
    QJsonObject status = m_daemon->getStatus();
    QCOMPARE(status.value("connected"), QJsonValue(false));
    QVERIFY(!status.contains("date"));
    QVERIFY(!m_wgutils->interfaceExists());
  }

  m_socket->disconnectFromServer();
  m_socket.reset(nullptr);
  m_daemon.reset(nullptr);
  m_dnsutils = nullptr;
  m_wgutils = nullptr;
}

void TestDaemon::send(const QJsonObject& msg) {
  QByteArray payload = QJsonDocument(msg).toJson(QJsonDocument::Compact);
  payload.append('\n');
  m_socket->write(payload);
  m_socket->flush();
}

// Receive precisely one JSON object from the socket.
bool TestDaemon::recv(QJsonObject& msg, int timeout) {
  QDeadlineTimer deadline(timeout);
  QEventLoop loop;

  while (!m_socket->canReadLine()) {
    if (deadline.hasExpired()) {
      return false;
    }
    Q_ASSERT(!deadline.hasExpired());
    loop.processEvents(QEventLoop::AllEvents, deadline);
  }

  QByteArray line = m_socket->readLine().trimmed();
  QJsonDocument json = QJsonDocument::fromJson(line);
  msg = json.object();
  return true;
}

QString TestDaemon::exampleKey(const char* prefix, char fill) {
  QByteArray key(prefix);
  key.resize((32 * 4 + 2) / 3, fill);
  key.append('=');
  return key;
}

InterfaceConfig TestDaemon::exampleConfig() {
  InterfaceConfig config;

  // Device configuration
  config.m_privateKey = exampleKey("TestPrivateKey");
  QByteArray key = config.m_privateKey.toUtf8();
  config.m_publicKey = QString::fromUtf8(Curve25519::generatePublicKey(key));
  config.m_deviceIpv4Address = "172.16.1.123/32";
  config.m_deviceIpv6Address = "fc02:1::dead:beef/128";

  // Wireguard server configuration.
  config.m_serverPublicKey = exampleKey("ServerPublicKey");
  config.m_exitCity = "Mordor";
  config.m_serverIpv4Gateway = "127.16.1.1";
  config.m_serverIpv6Gateway = "fc02::1/128";
  config.m_serverIpv4AddrIn = "10.123.45.67";
  config.m_serverIpv6AddrIn = "fc02:abcd::face:cafe";
  config.m_serverPort = 51820;

  // Misc configuration
  config.m_dnsServer = config.m_serverIpv4Gateway;
  config.m_allowedIPAddressRanges.append(IPAddress("0.0.0.0/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("172.16.0.0/16"));
  config.m_allowedIPAddressRanges.append(IPAddress("::/0"));
  config.m_allowedIPAddressRanges.append(IPAddress("fc02:abcd::/64"));

  return config;
}

// This should be implemented by moc-testlist.py
__attribute__((weak))
QList<const QMetaObject*> TestDaemon::testcases() {
  return QList<const QMetaObject*>();
}
