/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsroutemonitor.h"

#include <QScopeGuard>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("WindowsRouteMonitor");
};  // namespace

// Attempt to mark routing entries that we create with a relatively
// high metric. This ensures that we can skip over routes of our own
// creation when processing route changes, and ensures that we give
// way to other routing entries.
constexpr const ULONG EXCLUSION_ROUTE_METRIC = 0x5e72;

// Called by the kernel on route changes - perform some basic filtering and
// invoke the routeChanged slot to do the real work.
static void routeChangeCallback(PVOID context, PMIB_IPFORWARD_ROW2 row,
                                MIB_NOTIFICATION_TYPE type) {
  WindowsRouteMonitor* monitor = (WindowsRouteMonitor*)context;
  Q_UNUSED(type);

  // Ignore host route changes, and unsupported protocols.
  if (row->DestinationPrefix.Prefix.si_family == AF_INET6) {
    if (row->DestinationPrefix.PrefixLength >= 128) {
      return;
    }
  } else if (row->DestinationPrefix.Prefix.si_family == AF_INET) {
    if (row->DestinationPrefix.PrefixLength >= 32) {
      return;
    }
  } else {
    return;
  }
  // Ignore route changes that we created.
  if ((row->Protocol == MIB_IPPROTO_NETMGMT) &&
      (row->Metric == EXCLUSION_ROUTE_METRIC)) {
    return;
  }

  if (monitor->getLuid() != row->InterfaceLuid.Value) {
    QMetaObject::invokeMethod(monitor, "routeChanged", Qt::QueuedConnection);
  }
}

// Perform prefix matching comparison on IP addresses in host order.
static int prefixcmp(const void* a, const void* b, size_t bits) {
  size_t bytes = bits / 8;
  if (bytes > 0) {
    int diff = memcmp(a, b, bytes);
    if (diff != 0) {
      return diff;
    }
  }

  if (bits % 8) {
    quint8 avalue = *((const quint8*)a + bytes) >> (8 - bits % 8);
    quint8 bvalue = *((const quint8*)b + bytes) >> (8 - bits % 8);
    return avalue - bvalue;
  }

  return 0;
}

static bool routeContainsDest(const PMIB_IPFORWARD_ROW2 route,
                              const PMIB_IPFORWARD_ROW2 dest) {
  const IP_ADDRESS_PREFIX* routePrefix = &route->DestinationPrefix;
  const IP_ADDRESS_PREFIX* destPrefix = &dest->DestinationPrefix;
  if (routePrefix->Prefix.si_family != destPrefix->Prefix.si_family) {
    return false;
  }
  if (routePrefix->PrefixLength > destPrefix->PrefixLength) {
    return false;
  }
  if (routePrefix->Prefix.si_family == AF_INET) {
    return prefixcmp(&routePrefix->Prefix.Ipv4.sin_addr,
                    &destPrefix->Prefix.Ipv4.sin_addr,
                    routePrefix->PrefixLength) == 0;
  } if (routePrefix->Prefix.si_family == AF_INET6) {
    return prefixcmp(&routePrefix->Prefix.Ipv6.sin6_addr,
                    &destPrefix->Prefix.Ipv6.sin6_addr,
                    routePrefix->PrefixLength) == 0;
  } else {
    return false;
  }
}

WindowsRouteMonitor::WindowsRouteMonitor(quint64 luid, QObject* parent) :
    QObject(parent), m_luid(luid) {
  MZ_COUNT_CTOR(WindowsRouteMonitor);
  logger.debug() << "WindowsRouteMonitor created.";

  NotifyRouteChange2(AF_INET, routeChangeCallback, this, FALSE, &m_routeHandle);
}

WindowsRouteMonitor::~WindowsRouteMonitor() {
  MZ_COUNT_DTOR(WindowsRouteMonitor);
  CancelMibChangeNotify2(m_routeHandle);
  flushExclusionRoutes();
  logger.debug() << "WindowsRouteMonitor destroyed.";
}

void WindowsRouteMonitor::updateInterfaceMetrics(int family) {
  PMIB_IPINTERFACE_TABLE table;
  DWORD result = GetIpInterfaceTable(family, &table);
  if (result != NO_ERROR) {
    logger.warning() << "Failed to retrive interface table." << result;
    return;
  }
  auto guard = qScopeGuard([&] { FreeMibTable(table); });

  // Flush the list of interfaces that are valid for routing.
  if ((family == AF_INET) || (family == AF_UNSPEC)) {
    m_interfaceMetricsIpv4.clear();
  }
  if ((family == AF_INET6) || (family == AF_UNSPEC)) {
    m_interfaceMetricsIpv6.clear();
  }

  // Rebuild the list of interfaces that are valid for routing.
  for (ULONG i = 0; i < table->NumEntries; i++) {
    MIB_IPINTERFACE_ROW* row = &table->Table[i];
    if (row->InterfaceLuid.Value == m_luid) {
      continue;
    }
    if (!row->Connected) {
      continue;
    }

    if (row->Family == AF_INET) {
      logger.debug() << "Interface" << row->InterfaceIndex
                     << "is valid for IPv4 routing";
      m_interfaceMetricsIpv4[row->InterfaceLuid.Value] = row->Metric;
    }
    if (row->Family == AF_INET6) {
      logger.debug() << "Interface" << row->InterfaceIndex
                     << "is valid for IPv6 routing";
      m_interfaceMetricsIpv6[row->InterfaceLuid.Value] = row->Metric;
    }
  }
}

void WindowsRouteMonitor::updateExclusionRoute(MIB_IPFORWARD_ROW2* data,
                                               void* ptable) {
  PMIB_IPFORWARD_TABLE2 table = reinterpret_cast<PMIB_IPFORWARD_TABLE2>(ptable);
  SOCKADDR_INET nexthop = {};
  quint64 bestLuid = 0;
  int bestMatch = -1;
  ULONG bestMetric = ULONG_MAX;

  nexthop.si_family = data->DestinationPrefix.Prefix.si_family;
  for (ULONG i = 0; i < table->NumEntries; i++) {
    MIB_IPFORWARD_ROW2* row = &table->Table[i];
    // Ignore routes into the VPN interface.
    if (row->InterfaceLuid.Value == m_luid) {
      continue;
    }
    if (row->DestinationPrefix.PrefixLength < bestMatch) {
      continue;
    }
    // Ignore routes of our own creation.
    if ((row->Protocol == data->Protocol) && (row->Metric == data->Metric)) {
      continue;
    }

    // Check if the routing table entry matches the destination.
    if (!routeContainsDest(row, data)) {
      continue;
    }

    // Compute the combined interface and routing metric.
    ULONG routeMetric = row->Metric;
    if (data->DestinationPrefix.Prefix.si_family == AF_INET6) {
      if (!m_interfaceMetricsIpv6.contains(row->InterfaceLuid.Value)) {
        continue;
      }
      routeMetric += m_interfaceMetricsIpv6[row->InterfaceLuid.Value];
    } else if (data->DestinationPrefix.Prefix.si_family == AF_INET) {
      if (!m_interfaceMetricsIpv4.contains(row->InterfaceLuid.Value)) {
        continue;
      }
      routeMetric += m_interfaceMetricsIpv4[row->InterfaceLuid.Value];
    } else {
      // Unsupported destination address family.
      continue;
    }
    if (routeMetric < row->Metric) {
      routeMetric = ULONG_MAX;
    }

    // Prefer routes with lower metric if we find multiple matches
    // with the same prefix length.
    if ((row->DestinationPrefix.PrefixLength == bestMatch) &&
        (routeMetric >= bestMetric)) {
      continue;
    }

    // If we got here, then this is the longest prefix match so far.
    memcpy(&nexthop, &row->NextHop, sizeof(SOCKADDR_INET));
    bestMatch = row->DestinationPrefix.PrefixLength;
    bestMetric = routeMetric;
    if (bestMatch == data->DestinationPrefix.PrefixLength) {
      bestLuid = 0; // Don't write to the table if we find an exact match.
    } else {
      bestLuid = row->InterfaceLuid.Value;
    }
  }

  // If neither the interface nor next-hop have changed, then do nothing.
  if ((data->InterfaceLuid.Value) == bestLuid &&
      memcmp(&nexthop, &data->NextHop, sizeof(SOCKADDR_INET)) == 0) {
    return;
  }

  // Delete the previous routing table entry, if any.
  if (data->InterfaceLuid.Value != 0) {
    DWORD result = DeleteIpForwardEntry2(data);
    if ((result != NO_ERROR) && (result != ERROR_NOT_FOUND)) {
      logger.error() << "Failed to delete route:" << result;
    }
  }

  // Update the routing table entry.
  data->InterfaceLuid.Value = bestLuid;
  memcpy(&data->NextHop, &nexthop, sizeof(SOCKADDR_INET));
  if (data->InterfaceLuid.Value != 0) {
    DWORD result = CreateIpForwardEntry2(data);
    if (result != NO_ERROR) {
      logger.error() << "Failed to update route:" << result;
    }
  }
}

bool WindowsRouteMonitor::addExclusionRoute(const IPAddress& prefix) {
  logger.debug() << "Adding exclusion route for"
                 << logger.sensitive(prefix.toString());

  if (m_exclusionRoutes.contains(prefix)) {
    logger.warning() << "Exclusion route already exists";
    return false;
  }

  // Allocate and initialize the MIB routing table row.
  MIB_IPFORWARD_ROW2* data = new MIB_IPFORWARD_ROW2;
  InitializeIpForwardEntry(data);
  if (prefix.address().protocol() == QAbstractSocket::IPv6Protocol) {
    Q_IPV6ADDR buf = prefix.address().toIPv6Address();

    memcpy(&data->DestinationPrefix.Prefix.Ipv6.sin6_addr, &buf, sizeof(buf));
    data->DestinationPrefix.Prefix.Ipv6.sin6_family = AF_INET6;
    data->DestinationPrefix.PrefixLength = prefix.prefixLength();
  } else {
    quint32 buf = prefix.address().toIPv4Address();

    data->DestinationPrefix.Prefix.Ipv4.sin_addr.s_addr = htonl(buf);
    data->DestinationPrefix.Prefix.Ipv4.sin_family = AF_INET;
    data->DestinationPrefix.PrefixLength = prefix.prefixLength();
  }
  data->NextHop.si_family = data->DestinationPrefix.Prefix.si_family;

  // Set the rest of the flags for a static route.
  data->ValidLifetime = 0xffffffff;
  data->PreferredLifetime = 0xffffffff;
  data->Metric = EXCLUSION_ROUTE_METRIC;
  data->Protocol = MIB_IPPROTO_NETMGMT;
  data->Loopback = false;
  data->AutoconfigureAddress = false;
  data->Publish = false;
  data->Immortal = false;
  data->Age = 0;

  PMIB_IPFORWARD_TABLE2 table;
  int family;
  if (prefix.address().protocol() == QAbstractSocket::IPv6Protocol) {
    family = AF_INET6;
  } else {
    family = AF_INET;
  }

  DWORD result = GetIpForwardTable2(family, &table);
  if (result != NO_ERROR) {
    logger.error() << "Failed to fetch routing table:" << result;
    delete data;
    return false;
  }
  updateInterfaceMetrics(family);
  updateExclusionRoute(data, table);
  FreeMibTable(table);

  m_exclusionRoutes[prefix] = data;
  return true;
}

bool WindowsRouteMonitor::deleteExclusionRoute(const IPAddress& prefix) {
  logger.debug() << "Deleting exclusion route for"
                 << logger.sensitive(prefix.address().toString());

  for (;;) {
    MIB_IPFORWARD_ROW2* data = m_exclusionRoutes.take(prefix);
    if (data == nullptr) {
      break;
    }

    DWORD result = DeleteIpForwardEntry2(data);
    if ((result != ERROR_NOT_FOUND) && (result != NO_ERROR)) {
      logger.error() << "Failed to delete route to"
                     << logger.sensitive(prefix.toString())
                     << "result:" << result;
    }
    delete data;
  }

  return true;
}

void WindowsRouteMonitor::flushExclusionRoutes() {
  for (auto i = m_exclusionRoutes.begin(); i != m_exclusionRoutes.end(); i++) {
    MIB_IPFORWARD_ROW2* data = i.value();
    DWORD result = DeleteIpForwardEntry2(data);
    if ((result != ERROR_NOT_FOUND) && (result != NO_ERROR)) {
      logger.error() << "Failed to delete route to"
                     << logger.sensitive(i.key().toString())
                     << "result:" << result;
    }
    delete data;
  }
  m_exclusionRoutes.clear();
}

void WindowsRouteMonitor::routeChanged() {
  logger.debug() << "Routes changed";

  PMIB_IPFORWARD_TABLE2 table;
  DWORD result = GetIpForwardTable2(AF_UNSPEC, &table);
  if (result != NO_ERROR) {
    logger.error() << "Failed to fetch routing table:" << result;
    return;
  }

  updateInterfaceMetrics(AF_UNSPEC);
  for (MIB_IPFORWARD_ROW2* data : m_exclusionRoutes) {
    updateExclusionRoute(data, table);
  }

  FreeMibTable(table);
}
