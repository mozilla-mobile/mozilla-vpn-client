/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsbypass.h"

#include <WS2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#include <netioapi.h>

#include <QAbstractSocket>
#include <QHostAddress>
#include <QScopeGuard>
#include <QUuid>

#include "socks5.h"

// Fixed GUID of the Wireguard NT driver.
constexpr const QUuid WIREGUARD_NT_GUID(0xf64063ab, 0xbfee, 0x4881, 0xbf, 0x79, 0x36, 0x6e, 0x4c, 0xc7, 0xba, 0x75);

// Called by the kernel on network interface changes.
// Runs in some unknown thread, so invoke a Qt signal to do the real work.
static void netChangeCallback(PVOID context, PMIB_IPINTERFACE_ROW row,
                              MIB_NOTIFICATION_TYPE type) {
  WindowsBypass* bypass = static_cast<WindowsBypass*>(context);
  Q_UNUSED(type);

  // Invoke the route changed signal to do the real work in Qt.
  QMetaObject::invokeMethod(bypass, "refreshInterfaces", Qt::QueuedConnection);
}

// Called by the kernel on route changes.
// Runs in some unknown thread, so invoke a Qt signal to do the real work.
static void routeChangeCallback(PVOID context, PMIB_IPFORWARD_ROW2 row,
                                MIB_NOTIFICATION_TYPE type) {
  WindowsBypass* bypass = static_cast<WindowsBypass*>(context);
  Q_UNUSED(type);

  // Ignore routing changes into the Wireguard tunnel.
  int family = AF_UNSPEC;
  if (row) {
    GUID rowGuid;
    if (ConvertInterfaceLuidToGuid(&row->InterfaceLuid, &rowGuid) != NO_ERROR) {
      return;
    }
    if (QUuid(rowGuid) == WIREGUARD_NT_GUID) {
      return;
    }
    family = row->DestinationPrefix.Prefix.si_family;
  }

  // Invoke the route changed signal to do the real work in Qt.
  QMetaObject::invokeMethod(bypass, "refreshRoutes", Qt::QueuedConnection,
                            Q_ARG(int, family));
}

WindowsBypass::WindowsBypass(Socks5* proxy) : QObject(proxy) {
  connect(proxy, &Socks5::outgoingConnection, this,
          &WindowsBypass::outgoingConnection);

  NotifyIpInterfaceChange(AF_UNSPEC, netChangeCallback, this, true,
                          &m_netChangeHandle);
  NotifyRouteChange2(AF_UNSPEC, routeChangeCallback, this, true,
                     &m_routeChangeHandle);
}

WindowsBypass::~WindowsBypass() {
  CancelMibChangeNotify2(m_netChangeHandle);
  CancelMibChangeNotify2(m_routeChangeHandle);
}

void WindowsBypass::outgoingConnection(QAbstractSocket* s, const QHostAddress& dest) {
  const MIB_IPFORWARD_ROW2* route = lookupRoute(dest);
  if (route == nullptr) {
    // No routing exclusions to apply.
    return;
  }

  char buffer[NDIS_IF_MAX_STRING_SIZE+1];
  ConvertInterfaceLuidToNameA(&route->InterfaceLuid, buffer, sizeof(buffer));
  
  qDebug() << "Routing" << dest.toString() << "via" << buffer;

  // TODO: Interface binding shenanigans.
  // The magic goes here.
  return;
}

// static
QString WindowsBypass::win32strerror(unsigned long code) {
  LPWSTR buffer = nullptr;
  DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD size = FormatMessageW(flags, nullptr, code,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPWSTR)&buffer, 0, nullptr);
  QString result = QString::fromWCharArray(buffer, size);
  LocalFree(buffer);
  return result;
}

// Refresh our understanding of the current interfaces, and identify the VPN
// tunnel, if running.
void WindowsBypass::refreshInterfaces() {
  // Fetch the interface table.
  MIB_IPINTERFACE_TABLE* table;
  DWORD result = GetIpInterfaceTable(AF_UNSPEC, &table);
  if (result != NO_ERROR) {
    qWarning() << "GetIpInterfaceTable() failed:" << win32strerror(result);
    return;
  }
  auto guard = qScopeGuard([table]() { FreeMibTable(table); });

  m_vpnInterfaceIndex = -1;
  for (ULONG i = 0; i < table->NumEntries; i++) {
    const MIB_IPINTERFACE_ROW* row = &table->Table[i];
    GUID rowGuid;
    if (ConvertInterfaceLuidToGuid(&row->InterfaceLuid, &rowGuid) != NO_ERROR) {
      continue;
    }

    if (QUuid(rowGuid) == WIREGUARD_NT_GUID) {
      m_vpnInterfaceIndex = row->InterfaceIndex;
    }
  }

  if (m_vpnInterfaceIndex > 0) {
    qInfo() << "VPN tunnel is up:" << m_vpnInterfaceIndex;
  } else {
    qInfo() << "VPN tunnel is down";
  }
}

// In this function, we basically try our best to re-implement the Windows
// lookup algorithm, but instead we use our own local copy of the table.
// This returns the best route to the destination, or a null pointer if
// we can't find a matching route.
const MIB_IPFORWARD_ROW2* WindowsBypass::lookupRoute(const QHostAddress& dest) const {
  int bestLength = -1;
  ULONG bestMetric = ULONG_MAX;
  const MIB_IPFORWARD_ROW2* bestMatch = nullptr;
  const QVector<MIB_IPFORWARD_ROW2>* table;
  int family;

  if (dest.protocol() == QAbstractSocket::IPv4Protocol) {
    family = AF_INET;
    table = &m_routeTableIpv4;
  } else {
    family = AF_INET6;
    table = &m_routeTableIpv6;
  }

  for (qsizetype i = 0; i < table->count(); i++) {
    const MIB_IPFORWARD_ROW2& row = table->at(i);
    if (Q_UNLIKELY(row.DestinationPrefix.Prefix.si_family != family)) {
      continue;
    }
    if (row.DestinationPrefix.PrefixLength < bestLength) {
      continue;
    }

    // Find the route with the longest prefix match and lowest matric.
    QHostAddress prefix;
    if (family == AF_INET) {
      prefix.setAddress(row.DestinationPrefix.Prefix.Ipv4.sin_addr.s_addr);
    } else {
      prefix.setAddress(row.DestinationPrefix.Prefix.Ipv6.sin6_addr.s6_addr);
    }
    if (!dest.isInSubnet(prefix, row.DestinationPrefix.PrefixLength)) {
      continue;
    }
    // TODO: Need to check interface metrics too for proper tiebreaking.
    if ((row.DestinationPrefix.PrefixLength == bestLength) && (row.Metric < bestMetric)) {
      continue;
    }
    
    // This is the best route so far.
    bestLength = row.DestinationPrefix.PrefixLength;
    bestMetric = row.Metric;
    bestMatch = &row;
  }

  return bestMatch;
}

void WindowsBypass::updateTable(QVector<MIB_IPFORWARD_ROW2>& table, int family) {
  // Get the LUID of the wireguard interface, if it's up.
  NET_LUID vpnInterfaceLuid;
  GUID vpnInterfaceGuid = WIREGUARD_NT_GUID;
  if (ConvertInterfaceGuidToLuid(&vpnInterfaceGuid, &vpnInterfaceLuid) != NO_ERROR) {
    // If the interface is not up, then don't have to do any manipulation.
    table.clear();
    return;
  }

  // Fetch the routing table.
  MIB_IPFORWARD_TABLE2* mib;
  DWORD result = GetIpForwardTable2(family, &mib);
  if (result != NO_ERROR) {
    qWarning() << "GetIpForwardTable2() failed:" << win32strerror(result);
    table.clear();
    return;
  }
  auto guard = qScopeGuard([mib] { FreeMibTable(mib); });

  // First pass: iterate over the table and estimate the size to allocate.
  ULONG tableSize = 0;
  for (ULONG i = 0; i < mib->NumEntries; i++) {
    if (mib->Table[i].InterfaceLuid.Value != vpnInterfaceLuid.Value) {
      tableSize++;
    }
  }
  // If there were no routes to exclude, then we disable routing exclusions.
  if (tableSize == mib->NumEntries) {
    table.clear();
    return;
  }

  // Allocate memory for the table populate it with the entries which do not
  // route into the VPN.
  table.clear();
  table.reserve(tableSize);
  for (ULONG i = 0; i < mib->NumEntries; i++) {
    if (mib->Table[i].InterfaceLuid.Value != vpnInterfaceLuid.Value) {
      table.append(mib->Table[i]);
    }
  }
}

void WindowsBypass::refreshRoutes(int family) {
  if (family == AF_UNSPEC) {
    updateTable(m_routeTableIpv4, AF_INET);
    updateTable(m_routeTableIpv6, AF_INET6);
  } else if (family == AF_INET) {
    updateTable(m_routeTableIpv4, AF_INET);
  } else if (family == AF_INET6) {
    updateTable(m_routeTableIpv6, AF_INET6);
  }
}
