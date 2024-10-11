/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsbypass.h"

#include <WS2tcpip.h>
#include <netioapi.h>
#include <windows.h>
#include <winsock2.h>
#include <fwpmu.h>

#include <QAbstractSocket>
#include <QFileInfo>
#include <QHostAddress>
#include <QScopeGuard>
#include <QUuid>

#include "socks5.h"

#pragma comment(lib, "Fwpuclnt")

// Fixed GUID of the Wireguard NT driver.
constexpr const QUuid WIREGUARD_NT_GUID(0xf64063ab, 0xbfee, 0x4881, 0xbf, 0x79,
                                        0x36, 0x6e, 0x4c, 0xc7, 0xba, 0x75);

// Fixed GUID of the VPN Killswitch firewall sublayer
constexpr const QUuid KILLSWITCH_FW_GUID(0xc78056ff, 0x2bc1, 0x4211, 0xaa, 0xdd,
                                         0x7f, 0x35, 0x8d, 0xef, 0x20, 0x2d);

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
  
  // Watch for changes to the firewall
  FWPM_SESSION0 session;
  memset(&session, 0, sizeof(session));
  session.flags = FWPM_SESSION_FLAG_DYNAMIC;
  DWORD result = FwpmEngineOpen0(nullptr, RPC_C_AUTHN_WINNT, nullptr,
                                 &session, &m_fwEngineHandle);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Failed to open firewall engine:" << win32strerror(result);
    return;
  }

  GUID fwguid = KILLSWITCH_FW_GUID;
  FWPM_SUBLAYER_ENUM_TEMPLATE0 fwmatch { .providerKey = &fwguid };
  FWPM_SUBLAYER_SUBSCRIPTION0 fwsub = {0};
  fwsub.enumTemplate = &fwmatch;
  fwsub.flags = FWPM_SUBSCRIPTION_FLAG_NOTIFY_ON_ADD;
  fwsub.sessionKey = session.sessionKey;
  result = FwpmSubLayerSubscribeChanges0(m_fwEngineHandle, &fwsub,
                                         &WindowsBypass::setupFirewall, this,
                                         &m_fwChangeHandle);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Failed to create firewall subscription:" << win32strerror(result);
    return;
  }

  // If the sublayer already exists - immediately generate a notification.
  FWPM_SUBLAYER0* fwlayer;
  result = FwpmSubLayerGetByKey0(m_fwEngineHandle, &fwguid, &fwlayer);
  if (result == ERROR_SUCCESS) {
    FWPM_SUBLAYER_CHANGE0 change;
    change.changeType = FWPM_CHANGE_ADD;
    change.subLayerKey = KILLSWITCH_FW_GUID;
    setupFirewall(this, &change);
    FwpmFreeMemory0((void**)&fwlayer);
  }

}

WindowsBypass::~WindowsBypass() {
  CancelMibChangeNotify2(m_netChangeHandle);
  CancelMibChangeNotify2(m_addrChangeHandle);
  CancelMibChangeNotify2(m_routeChangeHandle);
  if (m_fwEngineHandle) {
    FwpmSubLayerUnsubscribeChanges0(m_fwEngineHandle, m_fwChangeHandle);
    FwpmEngineClose0(m_fwEngineHandle);
  }
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

// static
QString WindowsBypass::win32strerror(unsigned long code) {
  LPWSTR buffer = nullptr;
  DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD size = FormatMessageW(flags, nullptr, code,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPWSTR)&buffer, 0, nullptr);
  QString result = QString::fromWCharArray(buffer, size);
  LocalFree(buffer);
  return result;
}

quint64 WindowsBypass::getVpnLuid() const {
  // Get the LUID of the wireguard interface, if it's up.
  NET_LUID luid;
  GUID vpnInterfaceGuid = WIREGUARD_NT_GUID;
  if (ConvertInterfaceGuidToLuid(&vpnInterfaceGuid, &luid) != NO_ERROR) {
    return 0;
  }
  return luid.Value;
}

void WindowsBypass::refreshAddresses() {
  // Get the unicast address table.
  MIB_UNICASTIPADDRESS_TABLE* table;
  DWORD result = GetUnicastIpAddressTable(AF_UNSPEC, &table);
  if (result != NO_ERROR) {
    qWarning() << "GetUnicastIpAddressTable() failed:" << win32strerror(result);
    return;
  }
  auto guard = qScopeGuard([table]() { FreeMibTable(table); });

  // Populate entries.
  QHash<quint64, InterfaceData> data;
  const quint64 vpnInterfaceLuid = getVpnLuid();
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

  // Fetch the interface metrics too.
  for (auto i = data.begin(); i != data.end(); i++) {
    MIB_IPINTERFACE_ROW row = {0};
    row.InterfaceLuid.Value = i.key();
    if (GetIpInterfaceEntry(&row) == NO_ERROR) {
      i->metric = row.Metric;
    } else {
      i->metric = ULONG_MAX;
    }
  }

  // Swap the updated table into use.
  m_interfaceData.swap(data);
}

void WindowsBypass::interfaceChanged(quint64 luid) {
  qDebug() << "Interface changed for:" << QString::number(luid, 16);

  auto i = m_interfaceData.find(luid);
  if (i == m_interfaceData.end()) {
    // Nothing to update.
    return;
  }

  // Update the interface metric.
  MIB_IPINTERFACE_ROW row = {0};
  row.InterfaceLuid.Value = luid;
  if (GetIpInterfaceEntry(&row) == NO_ERROR) {
    i->metric = row.Metric;
  } else {
    i->metric = ULONG_MAX;
  }
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
    if (family == AF_INET) {
      if (ifData.ipv4addr.isNull()) {
        continue;
      }
    } else {
      if (ifData.ipv6addr.isNull()) {
        continue;
      }
    }

    // Choose the route with the best metric in case of a tie.
    ULONG rowMetric = row.Metric + ifData.metric;
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
    qWarning() << "GetIpForwardTable2() failed:" << win32strerror(result);
    return;
  }
  auto mibGuard = qScopeGuard([mib] { FreeMibTable(mib); });

  // First pass: iterate over the table and estimate the size to allocate.
  const quint64 vpnInterfaceLuid = getVpnLuid();
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

void WindowsBypass::setupFirewall(void* context,
                                  const FWPM_SUBLAYER_CHANGE0* change) {
  // Ignore everything except sublayer creation.
  if (change->changeType != FWPM_CHANGE_ADD) {
    return;
  }
  if (change->subLayerKey != KILLSWITCH_FW_GUID) {
    return;
  }

  // Get the AppID for the current executable;
  FWP_BYTE_BLOB* appID = NULL;
  WCHAR filePath[MAX_PATH];
  GetModuleFileNameW(nullptr, filePath, MAX_PATH);
  DWORD result = FwpmGetAppIdFromFileName0(filePath, &appID);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Firewall setup failed:" << win32strerror(result);
    return;
  }
  auto guard = qScopeGuard([appID]() { FwpmFreeMemory0((void**)&appID); });

  // Condition: Request must come from the .exe
  FWPM_FILTER_CONDITION0 conds;
  conds.fieldKey = FWPM_CONDITION_ALE_APP_ID;
  conds.matchType = FWP_MATCH_EQUAL;
  conds.conditionValue.type = FWP_BYTE_BLOB_TYPE;
  conds.conditionValue.byteBlob = appID;

  // Assemble the Filter base
  WCHAR filterName[] = L"Permit socksproxy bypass traffic";
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = &conds;
  filter.numFilterConditions = 1;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = 15;
  filter.subLayerKey = KILLSWITCH_FW_GUID;
  filter.flags = FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT;
  filter.displayData.name = filterName;

  // Start a transaction so that the firewall changes can be made asynchronously.
  HANDLE handle = reinterpret_cast<WindowsBypass*>(context)->m_fwEngineHandle;
  FwpmTransactionBegin0(handle, 0);
  auto txnguard = qScopeGuard([handle]() { FwpmTransactionAbort0 (handle); });

  WCHAR descv4out[] = L"Permit outbound IPv4 traffic from proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
  filter.displayData.description = descv4out;
  if (FwpmFilterAdd0(handle, &filter, nullptr, nullptr) != ERROR_SUCCESS) {
    return;
  }

  WCHAR descv4in[] = L"Permit inbound IPv4 traffic to proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
  filter.displayData.description = descv4in;
  if (FwpmFilterAdd0(handle, &filter, nullptr, nullptr) != ERROR_SUCCESS) {
    return;
  }

  WCHAR descv6out[] = L"Permit outbound IPv6 traffic from proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
  filter.displayData.description = descv6out;
  if (FwpmFilterAdd0(handle, &filter, nullptr, nullptr) != ERROR_SUCCESS) {
    return;
  }

  WCHAR descv6in[] = L"Permit inbound IPv6 traffic to proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;
  filter.displayData.description = descv6in;
  if (FwpmFilterAdd0(handle, &filter, nullptr, nullptr) != ERROR_SUCCESS) {
    return;
  }

  // Commit the transaction
  if (FwpmTransactionCommit0(handle) == ERROR_SUCCESS) {
    txnguard.dismiss();
  } else {
    qDebug() << "Firewall setup failed:" << win32strerror(result);
  }
}
