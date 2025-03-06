/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsbypass.h"

#include <WS2tcpip.h>
#include <fwpmu.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <windows.h>
#include <winsock2.h>

#include <QAbstractSocket>
#include <QFileInfo>
#include <QHostAddress>
#include <QScopeGuard>
#include <QSettings>
#include <QUuid>
#include <algorithm>
#include <ranges>

#include "socks5.h"
#include "winutils.h"

// Called by the kernel on network interface changes.
// Runs in some unknown thread, so invoke a Qt signal to do the real work.
static void netChangeCallback(PVOID context, PMIB_IPINTERFACE_ROW row,
                              MIB_NOTIFICATION_TYPE type) {
  WindowsBypass* bypass = static_cast<WindowsBypass*>(context);
  Q_UNUSED(type);
  if (row) {
    QMetaObject::invokeMethod(bypass, "interfaceChanged", Qt::QueuedConnection,
                              Q_ARG(quint64, row->InterfaceLuid.Value));
  }
}

// Called by the kernel on unicast address changes.
// Runs in some unknown thread, so invoke a Qt signal to do the real work.
static void addrChangeCallback(PVOID context, PMIB_UNICASTIPADDRESS_ROW row,
                               MIB_NOTIFICATION_TYPE type) {
  WindowsBypass* bypass = static_cast<WindowsBypass*>(context);
  Q_UNUSED(type);

  // Invoke the address changed signal to do the real work in Qt.
  QMetaObject::invokeMethod(bypass, "refreshAddresses", Qt::QueuedConnection);
}

// Called by the kernel on route changes.
// Runs in some unknown thread, so invoke a Qt signal to do the real work.
static void routeChangeCallback(PVOID context, PMIB_IPFORWARD_ROW2 row,
                                MIB_NOTIFICATION_TYPE type) {
  WindowsBypass* bypass = static_cast<WindowsBypass*>(context);
  Q_UNUSED(type);

  // Pass the address family to the route change signal too.
  int family = AF_UNSPEC;
  if (row) {
    family = row->DestinationPrefix.Prefix.si_family;
  }

  // Invoke the route changed signal to do the real work in Qt.
  QMetaObject::invokeMethod(bypass, "refreshRoutes", Qt::QueuedConnection,
                            Q_ARG(int, family));
}

WindowsBypass::WindowsBypass(Socks5* proxy) : QObject(proxy) {
  connect(proxy, &Socks5::outgoingConnection, this,
          &WindowsBypass::outgoingConnection);

  NotifyIpInterfaceChange(AF_UNSPEC, netChangeCallback, this, false,
                          &m_netChangeHandle);
  NotifyUnicastIpAddressChange(AF_UNSPEC, addrChangeCallback, this, true,
                               &m_addrChangeHandle);
  NotifyRouteChange2(AF_UNSPEC, routeChangeCallback, this, true,
                     &m_routeChangeHandle);
}

WindowsBypass::~WindowsBypass() {
  CancelMibChangeNotify2(m_netChangeHandle);
  CancelMibChangeNotify2(m_addrChangeHandle);
  CancelMibChangeNotify2(m_routeChangeHandle);
}

void WindowsBypass::outgoingConnection(QAbstractSocket* s,
                                       const QHostAddress& dest) {
  if (!dest.isGlobal() || dest.isUniqueLocalUnicast()) {
    // This destination should not require exclusion.
    return;
  }
  const MIB_IPFORWARD_ROW2* route = lookupRoute(dest);
  if (route == nullptr) {
    // No routing exclusions to apply.
    return;
  }

  // Find the accompanying source addresses.
  SOCKADDR_INET source = {0};
  const InterfaceData data = m_interfaceData.value(route->InterfaceLuid.Value);
  if (dest.protocol() == QAbstractSocket::IPv4Protocol) {
    if (data.ipv4addr.isNull()) {
      return;
    }
    source.Ipv4.sin_family = AF_INET;
    source.Ipv4.sin_port = 0;
    source.Ipv4.sin_addr.s_addr =
        qToBigEndian<quint32>(data.ipv4addr.toIPv4Address());
    qDebug() << "Routing" << dest.toString() << "via"
             << data.ipv4addr.toString();
  } else if (dest.protocol() == QAbstractSocket::IPv6Protocol) {
    if (data.ipv6addr.isNull()) {
      return;
    }
    Q_IPV6ADDR v6addr = data.ipv6addr.toIPv6Address();
    source.Ipv6.sin6_family = AF_INET6;
    source.Ipv6.sin6_port = 0;
    source.Ipv6.sin6_flowinfo = 0;
    source.Ipv6.sin6_scope_id = 0;  // TODO: Do we need to provide a scope?
    memcpy(&source.Ipv6.sin6_addr.s6_addr, &v6addr, 16);
    qDebug() << "Routing" << dest.toString() << "via"
             << data.ipv6addr.toString();
  } else {
    // Otherwise, this isn't an internet address we support.
    return;
  }

  // Create a new socket for this connection.
  SOCKET newsock = socket(source.si_family, SOCK_STREAM, IPPROTO_TCP);
  if (newsock == INVALID_SOCKET) {
    qWarning() << "socket creation failed:" << WSAGetLastError();
    return;
  }

  // Bind it to force its traffic to use a specific interface.
  int result =
      bind(newsock, reinterpret_cast<sockaddr*>(&source), sizeof(source));
  if (result != 0) {
    qWarning() << "socket bind failed:" << WSAGetLastError();
    closesocket(newsock);
    return;
  }

  // Provide the socket descriptor to this connection.
  if (!s->setSocketDescriptor(newsock, QAbstractSocket::UnconnectedState)) {
    qWarning() << "setSocketDescriptor() failed:" << s->errorString();
  }
}

void WindowsBypass::refreshAddresses() {
  // Get the unicast address table.
  MIB_UNICASTIPADDRESS_TABLE* table;
  DWORD result = GetUnicastIpAddressTable(AF_UNSPEC, &table);
  if (result != NO_ERROR) {
    qWarning() << "GetUnicastIpAddressTable() failed:"
               << WinUtils::win32strerror(result);
    return;
  }
  auto guard = qScopeGuard([table]() { FreeMibTable(table); });

  // Populate entries.
  QHash<quint64, InterfaceData> data;
  const quint64 vpnInterfaceLuid = WinUtils::getVpnLuid();
  for (ULONG i = 0; i < table->NumEntries; i++) {
    const MIB_UNICASTIPADDRESS_ROW* row = &table->Table[i];
    if (row->SkipAsSource) {
      continue;
    }
    if (row->InterfaceLuid.Value == vpnInterfaceLuid) {
      continue;
    }
    // Ignore everything except preferred addresses.
    if (row->DadState != IpDadStatePreferred) {
      continue;
    }

    QHostAddress addr;
    if (row->Address.si_family == AF_INET) {
      quint32 rowAddr = row->Address.Ipv4.sin_addr.s_addr;
      addr.setAddress(qFromBigEndian<quint32>(rowAddr));
    } else if (row->Address.si_family == AF_INET6) {
      addr.setAddress(row->Address.Ipv6.sin6_addr.s6_addr);
    } else {
      continue;
    }

    // Only use addresses that route globally.
    if (!addr.isGlobal() || addr.isUniqueLocalUnicast()) {
      continue;
    }
    // Ignore everything except preferred addresses.
    if (row->DadState != IpDadStatePreferred) {
      continue;
    }

    // Store the address.
    // TODO: Compare amongst multiple addresses on an interface.
    qDebug() << "Using " << addr.toString() << "for source address";
    if (row->Address.si_family == AF_INET) {
      data[row->InterfaceLuid.Value].ipv4addr = addr;
    } else {
      data[row->InterfaceLuid.Value].ipv6addr = addr;
    }
  }

  // Fetch the interface metrics.
  for (auto i = data.begin(); i != data.end(); i++) {
    MIB_IPINTERFACE_ROW row = {0};
    row.Family = AF_INET;
    row.InterfaceLuid.Value = i.key();
    if (GetIpInterfaceEntry(&row) == NO_ERROR) {
      i->ipv4metric = row.Metric;
    } else {
      i->ipv4metric = ULONG_MAX;
    }

    row.Family = AF_INET6;
    row.InterfaceLuid.Value = i.key();
    if (GetIpInterfaceEntry(&row) == NO_ERROR) {
      i->ipv6metric = row.Metric;
    } else {
      i->ipv6metric = ULONG_MAX;
    }
  }

  // Fetch the DNS resolvers too.
  QByteArray gaaBuffer(4096, 0);
  ULONG gaaBufferSize = gaaBuffer.size();
  auto adapterAddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(gaaBuffer.data());
  result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr,
                                adapterAddrs, &gaaBufferSize);
  if (result == ERROR_BUFFER_OVERFLOW) {
    gaaBuffer.resize(gaaBufferSize);
    adapterAddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(gaaBuffer.data());
    result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr,
                                  adapterAddrs, &gaaBufferSize);
  }
  for (auto i = adapterAddrs; result == NO_ERROR && i != nullptr; i = i->Next) {
    // Ignore DNS servers on interfaces without routable addresses.
    if (!data.contains(i->Luid.Value)) {
      continue;
    }
    if (i->FirstDnsServerAddress == nullptr) {
      continue;
    }

    data[i->Luid.Value].dnsAddr.clear();
    for (auto dns = i->FirstDnsServerAddress; dns != nullptr; dns = dns->Next) {
      struct sockaddr* sa = dns->Address.lpSockaddr;
      QHostAddress addr{sa};
      data[i->Luid.Value].dnsAddr.append(addr);
      qDebug() << "Adding" << addr.toString() << "as DNS server for "
               << i->Luid.Value;
    }
  }

  // Swap the updated table into use.
  m_interfaceData.swap(data);
  updateNameserver();
}

void WindowsBypass::interfaceChanged(quint64 luid) {
  qDebug() << "Interface changed for:" << QString::number(luid, 16);

  auto i = m_interfaceData.find(luid);
  if (i == m_interfaceData.end()) {
    // Nothing to update.
    return;
  }

  // Update the interface metrics.
  MIB_IPINTERFACE_ROW row = {0};
  row.Family = AF_INET;
  row.InterfaceLuid.Value = luid;
  if (GetIpInterfaceEntry(&row) == NO_ERROR) {
    i->ipv4metric = row.Metric;
  } else {
    i->ipv4metric = ULONG_MAX;
  }

  row.Family = AF_INET6;
  row.InterfaceLuid.Value = luid;
  if (GetIpInterfaceEntry(&row) == NO_ERROR) {
    i->ipv6metric = row.Metric;
  } else {
    i->ipv6metric = ULONG_MAX;
  }

  updateNameserver();
}

void WindowsBypass::updateNameserver() {
  // Update the preferred DNS server.
  struct keyed {
    QHostAddress addr;
    ulong metric;
  };
  QList<keyed> dnsNameservers;
  for (const auto& interfaceData : qAsConst(m_interfaceData)) {
    for (const auto& dnsAddr : qAsConst(interfaceData.dnsAddr)) {
      if (dnsAddr.protocol() == QAbstractSocket::IPv6Protocol) {
        dnsNameservers.append({dnsAddr, interfaceData.ipv6metric});
      } else {
        dnsNameservers.append({dnsAddr, interfaceData.ipv4metric});
      }
    }
  }
  // Sort the nameservers by it's metric smallest first
  std::ranges::sort(dnsNameservers,
                    [](auto a, auto b) { return a.metric < b.metric; });
  QList<QHostAddress> selectedNameServers(dnsNameservers.length());
  for (const auto& sortedDNSServer : qAsConst(dnsNameservers)) {
    selectedNameServers.append(sortedDNSServer.addr);
  }

  DNSResolver::instance()->setNameserver(selectedNameServers);
}

// In this function, we basically try our best to re-implement the Windows
// routing algorithm, but instead we use our own local copy of the table.
// This returns the best route to the destination, or a null pointer if
// we can't find a matching route.
const MIB_IPFORWARD_ROW2* WindowsBypass::lookupRoute(
    const QHostAddress& dest) const {
  int bestLength = -1;
  ULONG bestMetric = ULONG_MAX;
  const MIB_IPFORWARD_ROW2* bestMatch = nullptr;
  const QVector<MIB_IPFORWARD_ROW2>* table;
  int family;

  if (dest.protocol() == QAbstractSocket::IPv4Protocol) {
    family = AF_INET;
    table = &m_routeTableIpv4;
  } else if (dest.protocol() == QAbstractSocket::IPv6Protocol) {
    family = AF_INET6;
    table = &m_routeTableIpv6;
  } else {
    // This address is not routable.
    return nullptr;
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
      quint32 addr = row.DestinationPrefix.Prefix.Ipv4.sin_addr.s_addr;
      prefix.setAddress(qFromBigEndian<quint32>(addr));
    } else {
      prefix.setAddress(row.DestinationPrefix.Prefix.Ipv6.sin6_addr.s6_addr);
    }
    if (!dest.isInSubnet(prefix, row.DestinationPrefix.PrefixLength)) {
      continue;
    }

    // Ensure this route has a valid source address.
    auto ifData = m_interfaceData.value(row.InterfaceLuid.Value);
    ULONG rowMetric = row.Metric;
    if (family == AF_INET) {
      if (ifData.ipv4addr.isNull()) {
        continue;
      }
      rowMetric += ifData.ipv4metric;
    } else {
      if (ifData.ipv6addr.isNull()) {
        continue;
      }
      rowMetric += ifData.ipv6metric;
    }

    // Choose the route with the best metric in case of a tie.
    if (Q_UNLIKELY(rowMetric < row.Metric)) {
      rowMetric = ULONG_MAX;  // check for saturation arithmetic.
    }
    if ((row.DestinationPrefix.PrefixLength == bestLength) &&
        (rowMetric < bestMetric)) {
      continue;
    }

    // This is the best route so far.
    bestLength = row.DestinationPrefix.PrefixLength;
    bestMetric = rowMetric;
    bestMatch = &row;
  }

  return bestMatch;
}

void WindowsBypass::updateTable(QVector<MIB_IPFORWARD_ROW2>& table,
                                int family) {
  // Update the output table on exit.
  QVector<MIB_IPFORWARD_ROW2> update;
  auto swapGuard = qScopeGuard([&] { table.swap(update); });

  // Fetch the routing table.
  MIB_IPFORWARD_TABLE2* mib;
  DWORD result = GetIpForwardTable2(family, &mib);
  if (result != NO_ERROR) {
    qWarning() << "GetIpForwardTable2() failed:"
               << WinUtils::win32strerror(result);
    return;
  }
  auto mibGuard = qScopeGuard([mib] { FreeMibTable(mib); });

  // First pass: iterate over the table and estimate the size to allocate.
  const quint64 vpnInterfaceLuid = WinUtils::getVpnLuid();
  ULONG tableSize = 0;
  for (ULONG i = 0; i < mib->NumEntries; i++) {
    if (mib->Table[i].InterfaceLuid.Value != vpnInterfaceLuid) {
      tableSize++;
    }
  }
  // If there were no routes to exclude, then we can disable routing exclusions
  // by returning an empty routing table.
  if (tableSize == mib->NumEntries) {
    return;
  }

  // Allocate memory for the table populate it with the entries which do not
  // route into the VPN.
  update.reserve(tableSize);
  for (ULONG i = 0; i < mib->NumEntries; i++) {
    if (mib->Table[i].InterfaceLuid.Value != vpnInterfaceLuid) {
      update.append(mib->Table[i]);
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
