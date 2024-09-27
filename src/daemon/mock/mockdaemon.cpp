/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mockdaemon.h"

#include <QLocalSocket>
#include <QRandomGenerator>

#include "daemon/daemonlocalserverconnection.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("MockDaemon");
MockDaemon* s_daemon = nullptr;
}  // namespace

MockDaemon::MockDaemon(QObject* parent)
    : MockDaemon(
          "mozillavpn-mock-" +
              QString::number(QRandomGenerator::global()->generate64(), 16),
          parent) {}

MockDaemon::MockDaemon(const QString& name, QObject* parent)
    : Daemon(parent), m_socketName(name) {
  MZ_COUNT_CTOR(MockDaemon);

  logger.debug() << "Mock daemon created";

#ifndef MZ_WASM
  m_server.setSocketOptions(QLocalServer::UserAccessOption);
  if (!m_server.listen(m_socketName)) {
    logger.error() << "Failed to create mock daemon socket:"
                   << m_server.errorString();
  } else {
    logger.info() << "Listening at:" << m_server.fullServerName();
  }
  connect(&m_server, &QLocalServer::newConnection, this, [&]() {
    logger.debug() << "New connection received";
    if (!m_server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);
    new DaemonLocalServerConnection(this, socket);
  });
#endif

  m_dnsutils = new DnsUtilsMock(this);
  m_wgutils = new WireguardUtilsMock(this);

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;
}

MockDaemon::~MockDaemon() {
  MZ_COUNT_DTOR(MockDaemon);

  logger.debug() << "Mock daemon released";

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

bool MockDaemon::activate(const InterfaceConfig& config) {
  // Overwrite the addresses with loopback.
  InterfaceConfig mockConfig(config);
  mockConfig.m_serverIpv4Gateway = "127.0.0.1";
  mockConfig.m_serverIpv6Gateway = "::1";
  mockConfig.m_deviceIpv4Address = "127.0.0.1";
  mockConfig.m_deviceIpv6Address = "::1";

  return Daemon::activate(mockConfig);
}
