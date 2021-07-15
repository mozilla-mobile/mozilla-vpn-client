/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilswindows.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "windowsdaemon.h"
#include "wgquickprocess.h"

#include <QScopeGuard>

#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WireguardUtilsWindows");
}; // namespace

WireguardUtilsWindows::WireguardUtilsWindows(QObject* parent)
    : WireguardUtils(parent), m_tunnel(this) {
  MVPN_COUNT_CTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows created.";

  connect(&m_tunnel, &WindowsTunnelService::backendFailure, this,
          [&] { emit backendFailure(); });
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MVPN_COUNT_DTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows destroyed.";
}

WireguardUtils::peerBytes WireguardUtilsWindows::getThroughputForInterface() {
  peerBytes pb = {0,0};
  QString reply = m_tunnel.uapiCommand("get=1");

  for (const QString& line : reply.split('\n')) {
    if (!line.contains('=')) {
      continue;
    }

    QList<QString> parts = line.split('=');
    if (parts[0] == "tx_bytes") {
      pb.txBytes = parts[1].toDouble();
    } else if (parts[0] == "rx_bytes") {
      pb.rxBytes = parts[1].toDouble();
    }
  }
  
  return pb;
}

bool WireguardUtilsWindows::addInterface(const InterfaceConfig& config) {
  QString tunnelFile = WindowsCommons::tunnelConfigFile();
  if (tunnelFile.isEmpty()) {
    logger.log() << "Failed to choose the tunnel config file";
    return false;
  }

  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  if (!WgQuickProcess::createConfigFile(
          tunnelFile, config.m_privateKey, config.m_deviceIpv4Address,
          config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
          config.m_serverIpv6Gateway, config.m_serverPublicKey,
          config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
          addresses.join(", "), config.m_serverPort, config.m_ipv6Enabled,
          config.m_dnsServer)) {
    logger.log() << "Failed to create a config file";
    return false;
  }

  m_tunnel.resetLogs();
  if (!m_tunnel.start(tunnelFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return false;
  }

  logger.log() << "Registration completed";
  //m_state = Active;
  return true;
}

bool WireguardUtilsWindows::deleteInterface() {
  m_tunnel.stop();
  return true;
}

// Dummy implementations for now
bool WireguardUtilsWindows::updateInterface(const InterfaceConfig& config) {
  QString message;
  {
    QTextStream out(&message);
    out << "set=1\n";
    out << "replace_peers=true\n";

    QByteArray publicKey =
        QByteArray::fromBase64(config.m_serverPublicKey.toLocal8Bit()).toHex();
    out << "public_key=" << publicKey << "\n";

    out << "endpoint=" << config.m_serverIpv4AddrIn << ":"
        << config.m_serverPort << "\n";

    for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
      out << "allowed_ip=" << ip.toString() << "\n";
    }
  }

  QString reply = m_tunnel.uapiCommand(message);
  logger.log() << "DATA:" << reply;
  return true;
}

bool WireguardUtilsWindows::addRoutePrefix(const IPAddressRange& prefix) {
  Q_UNUSED(prefix);
  return true;
}
