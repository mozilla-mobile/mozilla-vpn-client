/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsserverlookup.h"

// Note: the include order is important!
// clang-format off
#include <qassert.h>
#include <qhostaddress.h>
#include <qlogging.h>
#include <qobject.h>
#include <qscopeguard.h>
#include <winerror.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <windns.h>
#include <optional>
#include <ip2string.h>
#include <optional>
#include <string>

// clang-format on

#include <ares.h>

#include <QHostAddress>
#include <QtConcurrent/QtConcurrent>

#include "winutils.h"

namespace ARES {
std::once_flag flag_init;
ares_channel_t* channel = nullptr;

auto currentServer = std::make_unique<ares_addr_node>();

/* Callback that is called when DNS query is finished */
static void addrinfo_cb(void* arg, int status, int timeouts,
                        struct ares_addrinfo* result) {
  switch (status) {
    case ARES_ENOTIMP:
      qDebug() << "The ares library does not know how to find addresses of "
                  "type family. ";
    case ARES_ENOTFOUND:
      qDebug() << " The name was not found.";
    case ARES_ENOMEM:
      qDebug() << "Memory was exhausted.";
    case ARES_ESERVICE:
      qDebug() << "The textual service name provided could not be dereferenced "
                  "into a port. ";
    case ARES_EDESTRUCTION:
      qDebug() << "The name service channel channel is being destroyed; the "
                  "query will not be completed. ";
      return;
  }

  Q_ASSERT(status == ARES_SUCCESS);
  // This should be our Socks5Connection
  auto ctx = static_cast<QObject*>(arg);
  auto guard = qScopeGuard([&]() { ares_freeaddrinfo(result); });
  if (!result) {
    return;
  }
  for (auto node = result->nodes; node != NULL; node = node->ai_next) {
    if (node->ai_family != AF_INET && node->ai_family != AF_INET6) {
      continue;
    }
    QHostAddress target{node->ai_addr};
    QMetaObject::invokeMethod(ctx, "onHostnameResolved", Qt::QueuedConnection,
                              Q_ARG(QHostAddress, target));
  }
}

void init() {
  struct ares_options options;
  int optmask = 0;

  ares_library_init(ARES_LIB_INIT_ALL);

  memset(&options, 0, sizeof(options));
  optmask |= ARES_OPT_EVENT_THREAD;
  options.evsys = ARES_EVSYS_DEFAULT;

  if (ares_init_options(&ARES::channel, &options, optmask) != ARES_SUCCESS) {
    printf("c-ares initialization issue\n");
  }
  // Cleanup on Shutdown
  QObject::connect(qApp, &QCoreApplication::aboutToQuit, []() {
    ares_destroy(ARES::channel);
    ares_library_cleanup();
  });
}

}  // namespace ARES

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

constexpr auto MAX_RECURSION = 5u;

void resolve(const QString& hostname, std::optional<QHostAddress> nameServer,
             QObject* parent) {
  // Assert ares is initialized
  std::call_once(ARES::flag_init, ARES::init);
  if (ARES::channel == nullptr) {
    Q_ASSERT(ARES::channel);
    return;
  }
  if (nameServer.has_value()) {
    auto server_str = nameServer->toString().toStdString();
    auto res = ares_set_servers_csv(ARES::channel, server_str.c_str());
    Q_ASSERT(res == ARES_SUCCESS);
  }
  auto name = hostname.toStdString();
  struct ares_addrinfo_hints hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = ARES_AI_CANONNAME;
  ares_getaddrinfo(ARES::channel, name.c_str(), NULL, &hints, ARES::addrinfo_cb,
                   parent);
}
}  // namespace DNSServerLookup
