/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsserverlookup.h"

// Note: the include order is important!
// clang-format off
#include <winerror.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <windns.h>
#include <ip2string.h>
// clang-format on

#include <QHostAddress>
#include <QObject>
#include <QScopeGuard>
#include <optional>
#include <string>

#include "winutils.h"

namespace DNSServerLookup {
/**
 * In order to get a Non VPN NameServer this func does:
 * Iterate over all adapters and then it's attached DNS-Servers.
 * Find the DNS Server with the Smallest attached "Metric" and returns it.
 * The Mozilla VPN adapter will always be ignored.
 */

std::optional<QHostAddress> getLocalDNSName() {
  static QHostAddress localAddr = {};
  if (!localAddr.isNull()) {
    return localAddr;
  }
  std::vector<std::byte> buffer(15000);

  u_long neededSize = buffer.size();
  auto addresses_ptr = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
  // Call GetAdaptersAddresses until you succeed or it indicates you need a
  // bigger buffer
  auto ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL,
                                  addresses_ptr, &neededSize);

  if (ret == ERROR_BUFFER_OVERFLOW) {
    // Retry with bigger buffer
    buffer.resize(neededSize);
    addresses_ptr = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL,
                               addresses_ptr, &neededSize);
  }
  if (ret != ERROR_SUCCESS) {
    qWarning() << "Failed to fetch AdaptersAddresses";
    return {};
  }
  QHostAddress selectedDNS = {};
  ULONG currentMetric = ULONG_MAX;

  // It's a linked list, neat.
  for (PIP_ADAPTER_ADDRESSES netAdapter = addresses_ptr; netAdapter != NULL;
       netAdapter = netAdapter->Next) {
    if (netAdapter->Luid.Value == WinUtils::getVpnLuid()) {
      // Don't consider the Mozilla VPN DNS Server.
      continue;
    }
    qDebug() << "Checking Adapter: "
             << QString::fromStdWString(netAdapter->FriendlyName) << " "
             << netAdapter->Ipv4Metric << "/" << netAdapter->Ipv6Metric;

    // Go through the DNS server list
    for (PIP_ADAPTER_DNS_SERVER_ADDRESS pDnServer =
             netAdapter->FirstDnsServerAddress;
         pDnServer != NULL; pDnServer = pDnServer->Next) {
      QHostAddress dns(pDnServer->Address.lpSockaddr);

      if (dns.isLinkLocal()) {
        continue;
      }
      if (dns.isLoopback()) {
        continue;
      }
      const auto getMetric = [](const QHostAddress& dns,
                                const PIP_ADAPTER_ADDRESSES adpt) {
        if (dns.protocol() == QAbstractSocket::IPv4Protocol) {
          return adpt->Ipv4Metric;
        }
        if (dns.protocol() == QAbstractSocket::IPv6Protocol) {
          return adpt->Ipv6Metric;
        }
        return ULONG_MAX;
      };
      auto const routeMetric = getMetric(dns, netAdapter);
      if (routeMetric >= currentMetric) {
        // If the new dns server does not have a smaller metric, we
        // should not consider it
        continue;
      }
      currentMetric = routeMetric;
      selectedDNS = dns;
    }
  }
  if (selectedDNS.isNull()) {
    return {};
  }
  localAddr = selectedDNS;
  return selectedDNS;
}

}  // namespace DNSServerLookup
