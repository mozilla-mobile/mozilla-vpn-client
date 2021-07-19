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
  logger.log() << "WireguardUtilsWindows created.";

  connect(&m_tunnel, &WindowsTunnelService::backendFailure, this,
          [&] { emit backendFailure(); });
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MVPN_COUNT_DTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows destroyed.";
}

WireguardUtils::peerBytes WireguardUtilsWindows::getThroughputForInterface() {
  peerBytes pb = {0, 0};
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

  QMap<QString, QString> extraConfig;
  extraConfig["Table"] = "off";
  if (!WgQuickProcess::createConfigFile(tunnelFile, config, extraConfig)) {
    logger.log() << "Failed to create a config file";
    return false;
  }

  if (!m_tunnel.start(tunnelFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return false;
  }

  // Determine the interface LUID
  NET_LUID luid;
  QString ifAlias = QFileInfo(tunnelFile).baseName();
  DWORD result = ConvertInterfaceAliasToLuid((wchar_t*)ifAlias.utf16(), &luid);
  if (result != 0) {
    logger.log() << "Failed to lookup LUID:" << result;
    return false;
  }
  m_luid = luid.Value;

  logger.log() << "Registration completed";
  return true;
}

bool WireguardUtilsWindows::deleteInterface() {
  m_tunnel.stop();
  return true;
}

bool WireguardUtilsWindows::updateInterface(const InterfaceConfig& config) {
  // Update the interface config
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
  DWORD result;
  MIB_IPFORWARD_ROW2 entry;
  InitializeIpForwardEntry(&entry);

  // Populate the next hop
  if (prefix.type() == IPAddressRange::IPv6) {
    InetPtonA(AF_INET6, qPrintable(prefix.ipAddress()),
              &entry.DestinationPrefix.Prefix.Ipv6.sin6_addr);
    entry.DestinationPrefix.Prefix.Ipv6.sin6_family = AF_INET6;
    entry.DestinationPrefix.PrefixLength = prefix.range();
  } else {
    InetPtonA(AF_INET, qPrintable(prefix.ipAddress()),
              &entry.DestinationPrefix.Prefix.Ipv4.sin_addr);
    entry.DestinationPrefix.Prefix.Ipv4.sin_family = AF_INET;
    entry.DestinationPrefix.PrefixLength = prefix.range();
  }
  entry.InterfaceLuid.Value = m_luid;
  entry.NextHop.si_family = entry.DestinationPrefix.Prefix.si_family;

  // Set the rest of the flags for a static route.
  entry.ValidLifetime = 0xffffffff;
  entry.PreferredLifetime = 0xffffffff;
  entry.Metric = 0;
  entry.Protocol = MIB_IPPROTO_NETMGMT;
  entry.Loopback = false;
  entry.AutoconfigureAddress = false;
  entry.Publish = false;
  entry.Immortal = false;
  entry.Age = 0;

  // Install the route
  result = CreateIpForwardEntry2(&entry);
  if (result != NO_ERROR) {
    logger.log() << "Failed to create route to" << prefix.toString()
                 << "result:" << result;
  }
  return result == NO_ERROR;
}

void WireguardUtilsWindows::flushRoutes() {
  DWORD result;
  PMIB_IPFORWARD_TABLE2 table;

  // Fetch the routing table
  result = GetIpForwardTable2(AF_UNSPEC, &table);
  if (result != NO_ERROR) {
    logger.log() << "Failed to fetch route table:" << result;
    return;
  }
  auto guard = qScopeGuard([&] { FreeMibTable(table); });

  // Delete any entries matching our LUID.
  for (ULONG i = 0; i < table->NumEntries; i++) {
    if (table->Table[i].InterfaceLuid.Value == m_luid) {
      DeleteIpForwardEntry2(&table->Table[i]);
    }
  }
}
