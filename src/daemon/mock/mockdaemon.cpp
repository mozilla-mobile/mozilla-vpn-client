/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mockdaemon.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QtGlobal>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("MockDaemon");
MockDaemon* s_daemon = nullptr;
}  // namespace

MockDaemon::MockDaemon() : Daemon(nullptr) {
  MZ_COUNT_CTOR(MockDaemon);

  logger.debug() << "Mock daemon created";

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

// static
MockDaemon* MockDaemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
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
