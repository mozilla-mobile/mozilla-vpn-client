/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsserverlookup.h"

// Note: the include order is important!
// clang-format off
#include <qassert.h>
#include <qexception.h>
#include <qhostaddress.h>
#include <qlogging.h>
#include <qscopeguard.h>
#include <winerror.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <windns.h>
#include <optional>
#include <ip2string.h>
#include <string>

// clang-format on

#include <QHostAddress>

#include "winutils.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "ntdll.lib")

#include <QtConcurrent/QtConcurrent>

namespace DNSServerLookup {

/**
 * In order to get a Non VPN NameServer this func does:
 * Iterate over all adapters and then it's attached DNS-Servers.
 * Find the DNS Server with the Smallest attached "Metric" and returns it.
 * The Mozilla VPN adapter will always be ignored.
 */

std::optional<QHostAddress> getLocalDNSName() {
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
  return selectedDNS;
}

constexpr auto MAX_RECURSION = 5u;

QHostAddress queryDNS(const QString& hostname,
                      std::optional<QHostAddress> nameServer,
                      uint recursion = 0) {
  if (recursion > MAX_RECURSION) {
    return {};
  }
  std::wstring wideHost = hostname.toStdWString();

  DNS_ADDR_ARRAY* pServerList = nullptr;
  DNS_ADDR_ARRAY serverArray;
  ZeroMemory(&serverArray, sizeof(serverArray));

  if (nameServer.has_value()) {
    serverArray.MaxCount = 1;
    serverArray.AddrCount = 1;

    if (nameServer->protocol() == QAbstractSocket::IPv4Protocol) {
      // IPv4
      auto* sockAddrIn =
          reinterpret_cast<SOCKADDR_IN*>(&serverArray.AddrArray[0].MaxSa);
      sockAddrIn->sin_family = AF_INET;
      sockAddrIn->sin_port = 0;  // or htons(53)
      sockAddrIn->sin_addr.S_un.S_addr = htonl(nameServer->toIPv4Address());
    } else if (nameServer->protocol() == QAbstractSocket::IPv6Protocol) {
      // IPv6
      auto* sockAddrIn6 =
          reinterpret_cast<SOCKADDR_IN6*>(&serverArray.AddrArray[0].MaxSa);
      sockAddrIn6->sin6_family = AF_INET6;
      sockAddrIn6->sin6_port = 0;  // or htons(53)
      auto v6 = nameServer->toIPv6Address();
      memcpy(&sockAddrIn6->sin6_addr, &v6, sizeof(v6));
      // If needed, handle scope_id for link-local addresses
      sockAddrIn6->sin6_scope_id = nameServer->scopeId().toULong();
    }
    pServerList = &serverArray;
  }

  // 2) Build the DNS_QUERY_REQUEST
  DNS_QUERY_REQUEST request;
  ZeroMemory(&request, sizeof(request));
  request.Version = DNS_QUERY_REQUEST_VERSION1;
  request.QueryName = wideHost.c_str();
  request.QueryType = DNS_TYPE_ANY;
  request.QueryOptions = DNS_QUERY_BYPASS_CACHE;  // or DNS_QUERY_STANDARD
  request.pDnsServerList = pServerList;  // Our custom DNS server if provided

  // 3) Prepare the DNS_QUERY_RESULT
  DNS_QUERY_RESULT queryResult;
  ZeroMemory(&queryResult, sizeof(queryResult));
  queryResult.Version = DNS_QUERY_REQUEST_VERSION1;

  // 4) Perform a BLOCKING call to DnsQueryEx
  DNS_STATUS status = DnsQueryEx(&request, &queryResult, nullptr);

  if (status != ERROR_SUCCESS || !queryResult.pQueryRecords) {
    if (queryResult.pQueryRecords) {
      DnsFree(queryResult.pQueryRecords, DnsFreeRecordList);
    }
    switch (status) {
      case DNS_INFO_NO_RECORDS:
        throw DnsFetchException{"no such host was found"};
      case DNS_REQUEST_PENDING:
        qFatal() << "Wrong invocation, expected sync dns query";
      case ERROR_INVALID_PARAMETER:
        qFatal() << "Wrong invocation of DnsQueryEx";
        break;
      case DNS_ERROR_RCODE_REFUSED:
      case DNS_ERROR_RCODE_SERVER_FAILURE:
      case DNS_ERROR_RCODE_NO_ERROR:
        throw DnsFetchException{"SERVER_FAILURE"};
    }
  }
  auto leakGuard = qScopeGuard(
      [&]() { DnsFree(queryResult.pQueryRecords, DnsFreeRecordList); });
  // 5) Parse the returned DNS records
  for (PDNS_RECORD pRec = queryResult.pQueryRecords; pRec; pRec = pRec->pNext) {
    if (pRec->wType == DNS_TYPE_A) {
      // Convert from network byte order
      quint32 ipv4 = ntohl(pRec->Data.A.IpAddress);

      // Return the first found A-record
      QHostAddress result(ipv4);
      Q_ASSERT(!result.isNull());
      return result;
    }
    if (pRec->wType == DNS_TYPE_AAAA) {
      IN6_ADDR Ipv6address;

      WCHAR Ipv6String[64] = L"\0";

      CopyMemory(&Ipv6address, &pRec->Data.AAAA.Ip6Address,
                 sizeof(Ipv6address));
      RtlIpv6AddressToString(&Ipv6address, Ipv6String);

      auto result = QHostAddress{QString::fromStdWString(Ipv6String)};
      qDebug() << "Resolved : " << result.toString();
      Q_ASSERT(!result.isNull());
      return result;
    }
    if (pRec->wType == DNS_TYPE_CNAME) {
      auto cname = QString::fromStdWString(pRec->Data.CNAME.pNameHost);
      return queryDNS(cname, nameServer, recursion + 1);
    }
  }
  return {};
}

QFuture<QHostAddress> resolve(const QString& hostname,
                              std::optional<QHostAddress> nameServer) {
  // The lambda returns QHostAddress, so QtConcurrent::run returns
  // QFuture<QHostAddress>.
  return QtConcurrent::run([hostname, nameServer]() -> QHostAddress {
    // We'll try a few times if desired
    for (int attempt = 0; attempt < MAX_DNS_LOOKUP_ATTEMPTS; ++attempt) {
      auto host = queryDNS(hostname, nameServer);
      if (!host.isNull()) {
        return host;
      }
    }
    throw DnsFetchException{"no such host was found"};
  });
}
}  // namespace DNSServerLookup
