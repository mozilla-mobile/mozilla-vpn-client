/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "platforms/windows/windowsservicemanager.h"
#include "wgquickprocess.h"
#include "windowsfirewall.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QNetworkInterface>
#include <QScopeGuard>
#include <QTextStream>
#include <QtGlobal>

#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WindowsDaemon");
}

WindowsDaemon::WindowsDaemon() : Daemon(nullptr), m_splitTunnelManager(this) {
  MVPN_COUNT_CTOR(WindowsDaemon);

  m_wgutils = new WireguardUtilsWindows(this);

  connect(m_wgutils, &WireguardUtilsWindows::backendFailure, this,
          &WindowsDaemon::monitorBackendFailure);
}

WindowsDaemon::~WindowsDaemon() {
  MVPN_COUNT_DTOR(WindowsDaemon);
  logger.debug() << "Daemon released";
}
void WindowsDaemon::prepareActivation(const InterfaceConfig& config) {
  // Before creating the interface we need to check which adapter
  // routes to the server endpoint
  auto serveraddr = QHostAddress(config.m_serverIpv4AddrIn);
  m_inetAdapterIndex = WindowsCommons::AdapterIndexTo(serveraddr);
}

bool WindowsDaemon::run(Op op, const InterfaceConfig& config) {
  bool splitTunnelEnabled = config.m_vpnDisabledApps.length() > 0;

  if (op == Down) {
    if (splitTunnelEnabled) {
      m_splitTunnelManager.stop();
    }
    WindowsFirewall::instance()->disableKillSwitch();
    return true;
  }
  if (splitTunnelEnabled) {
    logger.debug() << "Tunnel UP, Starting SplitTunneling";
    if (!WindowsSplitTunnel::isInstalled()) {
      logger.warning() << "Split Tunnel Driver not Installed yet, fixing this.";
      WindowsSplitTunnel::installDriver();
    }
    m_splitTunnelManager.start(m_inetAdapterIndex);
    m_splitTunnelManager.setRules(config.m_vpnDisabledApps);
  }
  WindowsFirewall::instance()->enableKillSwitch(
      WindowsCommons::VPNAdapterIndex(), config);
  return true;
}

QByteArray WindowsDaemon::getStatus() {
  logger.debug() << "Status request";

  bool connected = m_connections.contains(0);
  QJsonObject obj;
  obj.insert("type", "status");
  obj.insert("connected", connected);

  if (connected) {
    const ConnectionState& state = m_connections.value(0).m_config;
    WireguardUtilsWindows::peerBytes pb =
        m_wgutils->getThroughputForInterface();
    obj.insert("serverIpv4Gateway", state.m_config.m_serverIpv4Gateway);
    obj.insert("deviceIpv4Address", state.m_config.m_deviceIpv4Address);
    obj.insert("date", state.m_date.toString());
    obj.insert("txBytes", QJsonValue(pb.txBytes));
    obj.insert("rxBytes", QJsonValue(pb.rxBytes));
  }

  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

bool WindowsDaemon::supportServerSwitching(
    const InterfaceConfig& config) const {
  if (!m_connections.contains(config.m_hopindex)) {
    return false;
  }
  const InterfaceConfig& current =
      m_connections.value(config.m_hopindex).m_config;

  return current.m_privateKey == config.m_privateKey &&
         current.m_deviceIpv4Address == config.m_deviceIpv4Address &&
         current.m_deviceIpv6Address == config.m_deviceIpv6Address &&
         current.m_serverIpv4Gateway == config.m_serverIpv4Gateway &&
         current.m_serverIpv6Gateway == config.m_serverIpv6Gateway;
}

void WindowsDaemon::monitorBackendFailure() {
  logger.warning() << "Tunnel service is down";

  emit backendFailure();
  deactivate();
}
