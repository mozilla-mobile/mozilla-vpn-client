/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "platforms/windows/windowsservicemanager.h"
#include "wgquickprocess.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
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
  logger.log() << "Daemon released";
}

bool WindowsDaemon::run(Op op, const InterfaceConfig& config) {
  bool splitTunnelEnabled = config.m_vpnDisabledApps.length() > 0;

  if (op == Down) {
    if (splitTunnelEnabled) {
      m_splitTunnelManager.stop();
    }
    return true;
  }
  if (splitTunnelEnabled) {
    logger.log() << "Tunnel UP, Starting SplitTunneling";
    if (!WindowsSplitTunnel::isInstalled()) {
      logger.log() << "Split Tunnel Driver not Installed yet, fixing this.";
      WindowsSplitTunnel::installDriver();
    }
    m_splitTunnelManager.start();
    m_splitTunnelManager.setRules(config.m_vpnDisabledApps);
  }
  return true;
}

QByteArray WindowsDaemon::getStatus() {
  logger.log() << "Status request";

  QJsonObject obj;
  obj.insert("type", "status");
  obj.insert("connected", m_connected);

  if (m_connected) {
    WireguardUtilsWindows::peerBytes pb =
        m_wgutils->getThroughputForInterface();
    obj.insert("serverIpv4Gateway", m_lastConfig.m_serverIpv4Gateway);
    obj.insert("deviceIpv4Address", m_lastConfig.m_deviceIpv4Address);
    obj.insert("date", m_connectionDate.toString());
    obj.insert("txBytes", QJsonValue(pb.txBytes));
    obj.insert("rxBytes", QJsonValue(pb.rxBytes));
  }

  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

bool WindowsDaemon::supportServerSwitching(
    const InterfaceConfig& config) const {
  return m_lastConfig.m_privateKey == config.m_privateKey &&
         m_lastConfig.m_deviceIpv4Address == config.m_deviceIpv4Address &&
         m_lastConfig.m_deviceIpv6Address == config.m_deviceIpv6Address &&
         m_lastConfig.m_serverIpv4Gateway == config.m_serverIpv4Gateway &&
         m_lastConfig.m_serverIpv6Gateway == config.m_serverIpv6Gateway;
}

void WindowsDaemon::monitorBackendFailure() {
  logger.log() << "Tunnel service is down";

  emit backendFailure();
  deactivate();
}
