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
#include <QFileInfo>

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

namespace {
Logger logger(LOG_WINDOWS, "WireguardUtilsWindows");
};  // namespace

WireguardUtilsWindows::WireguardUtilsWindows(QObject* parent)
    : WireguardUtils(parent), m_tunnel(this) {
  MVPN_COUNT_CTOR(WireguardUtilsWindows);
  logger.debug() << "WireguardUtilsWindows created.";

  connect(&m_tunnel, &WindowsTunnelService::backendFailure, this,
          [&] { emit backendFailure(); });
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MVPN_COUNT_DTOR(WireguardUtilsWindows);
  logger.debug() << "WireguardUtilsWindows destroyed.";
}

WireguardUtils::peerStatus WireguardUtilsWindows::getPeerStatus(
    const QString& pubkey) {
  peerStatus status = {0, 0};
  QString hexkey = QByteArray::fromBase64(pubkey.toUtf8()).toHex();
  QString reply = m_tunnel.uapiCommand("get=1");
  bool match = false;

  for (const QString& line : reply.split('\n')) {
    int eq = line.indexOf('=');
    if (eq <= 0) {
      continue;
    }
    QString name = line.left(eq);
    QString value = line.mid(eq + 1);

    if (name == "public_key") {
      match = (value == hexkey);
      continue;
    } else if (!match) {
      continue;
    }

    if (name == "tx_bytes") {
      status.txBytes = value.toDouble();
    }
    if (name == "rx_bytes") {
      status.rxBytes = value.toDouble();
    }
  }

  return status;
}

bool WireguardUtilsWindows::addInterface(const InterfaceConfig& config) {
  QString tunnelFile = WindowsCommons::tunnelConfigFile();
  if (tunnelFile.isEmpty()) {
    logger.error() << "Failed to choose the tunnel config file";
    return false;
  }

  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  QMap<QString, QString> extraConfig;
  extraConfig["Table"] = "off";
  if (!WgQuickProcess::createConfigFile(tunnelFile, config, extraConfig)) {
    logger.error() << "Failed to create a config file";
    return false;
  }

  if (!m_tunnel.start(tunnelFile)) {
    logger.error() << "Failed to activate the tunnel service";
    return false;
  }

  // Determine the interface LUID
  NET_LUID luid;
  QString ifAlias = QFileInfo(tunnelFile).baseName();
  DWORD result = ConvertInterfaceAliasToLuid((wchar_t*)ifAlias.utf16(), &luid);
  if (result != 0) {
    logger.error() << "Failed to lookup LUID:" << result;
    return false;
  }
  m_luid = luid.Value;

  logger.debug() << "Registration completed";
  return true;
}

bool WireguardUtilsWindows::deleteInterface() {
  m_tunnel.stop();
  return true;
}

bool WireguardUtilsWindows::updatePeer(const InterfaceConfig& config) {
  QByteArray publicKey =
      QByteArray::fromBase64(qPrintable(config.m_serverPublicKey));

  // Update/create the peer config
  QString message;
  QTextStream out(&message);
  out << "set=1\n";
  out << "public_key=" << QString(publicKey.toHex()) << "\n";
  if (!config.m_serverIpv4AddrIn.isNull()) {
    out << "endpoint=" << config.m_serverIpv4AddrIn << ":";
  } else if (!config.m_serverIpv6AddrIn.isNull()) {
    out << "endpoint=[" << config.m_serverIpv6AddrIn << "]:";
  } else {
    logger.warning() << "Failed to create peer with no endpoints";
    return false;
  }
  out << config.m_serverPort << "\n";

  out << "replace_allowed_ips=true\n";
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    out << "allowed_ip=" << ip.toString() << "\n";
  }

  QString reply = m_tunnel.uapiCommand(message);
  logger.debug() << "DATA:" << reply;
  return true;
}

bool WireguardUtilsWindows::deletePeer(const QString& pubkey) {
  QByteArray publicKey = QByteArray::fromBase64(qPrintable(pubkey));

  QString message;
  QTextStream out(&message);
  out << "set=1\n";
  out << "public_key=" << QString(publicKey.toHex()) << "\n";
  out << "remove=true\n";

  QString reply = m_tunnel.uapiCommand(message);
  logger.debug() << "DATA:" << reply;
  return true;
}

void WireguardUtilsWindows::buildMibForwardRow(const IPAddressRange& prefix,
                                               void* row) {
  MIB_IPFORWARD_ROW2* entry = (MIB_IPFORWARD_ROW2*)row;
  InitializeIpForwardEntry(entry);

  // Populate the next hop
  if (prefix.type() == IPAddressRange::IPv6) {
    InetPtonA(AF_INET6, qPrintable(prefix.ipAddress()),
              &entry->DestinationPrefix.Prefix.Ipv6.sin6_addr);
    entry->DestinationPrefix.Prefix.Ipv6.sin6_family = AF_INET6;
    entry->DestinationPrefix.PrefixLength = prefix.range();
  } else {
    InetPtonA(AF_INET, qPrintable(prefix.ipAddress()),
              &entry->DestinationPrefix.Prefix.Ipv4.sin_addr);
    entry->DestinationPrefix.Prefix.Ipv4.sin_family = AF_INET;
    entry->DestinationPrefix.PrefixLength = prefix.range();
  }
  entry->InterfaceLuid.Value = m_luid;
  entry->NextHop.si_family = entry->DestinationPrefix.Prefix.si_family;

  // Set the rest of the flags for a static route.
  entry->ValidLifetime = 0xffffffff;
  entry->PreferredLifetime = 0xffffffff;
  entry->Metric = 0;
  entry->Protocol = MIB_IPPROTO_NETMGMT;
  entry->Loopback = false;
  entry->AutoconfigureAddress = false;
  entry->Publish = false;
  entry->Immortal = false;
  entry->Age = 0;
}

bool WireguardUtilsWindows::updateRoutePrefix(const IPAddressRange& prefix,
                                              int hopindex) {
  Q_UNUSED(hopindex);
  MIB_IPFORWARD_ROW2 entry;
  buildMibForwardRow(prefix, &entry);

  // Install the route
  DWORD result = CreateIpForwardEntry2(&entry);
  if (result == ERROR_OBJECT_ALREADY_EXISTS) {
    return true;
  }
  if (result != NO_ERROR) {
    logger.error() << "Failed to create route to" << prefix.toString()
                   << "result:" << result;
  }
  return result == NO_ERROR;
}

bool WireguardUtilsWindows::deleteRoutePrefix(const IPAddressRange& prefix,
                                              int hopindex) {
  Q_UNUSED(hopindex);
  MIB_IPFORWARD_ROW2 entry;
  buildMibForwardRow(prefix, &entry);

  // Install the route
  DWORD result = DeleteIpForwardEntry2(&entry);
  if (result == ERROR_NOT_FOUND) {
    return true;
  }
  if (result != NO_ERROR) {
    logger.error() << "Failed to delete route to" << prefix.toString()
                   << "result:" << result;
  }
  return result == NO_ERROR;
}
