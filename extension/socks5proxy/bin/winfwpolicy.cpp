/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "winfwpolicy.h"

#include <WS2tcpip.h>
#include <fwpmu.h>
#include <iptypes.h>

#include <QScopeGuard>
#include <QSettings>
#include <QString>
#include <QTcpServer>
#include <QUuid>

#include "socks5.h"
#include "winutils.h"

#pragma comment(lib, "Fwpuclnt")

// Fixed GUID of the VPN Killswitch firewall sublayer
constexpr const QUuid KILLSWITCH_FW_GUID(0xc78056ff, 0x2bc1, 0x4211, 0xaa, 0xdd,
                                         0x7f, 0x35, 0x8d, 0xef, 0x20, 0x2d);

// Fixed GUID of our own firewall sublayer
constexpr const QUuid LOCALPROXY_FW_GUID(0x0555706c, 0x4468, 0x4ec6, 0xb4, 47,
                                         0x20, 0xe7, 0x4a, 0x10, 0x06, 0xe7);

WinFwPolicy* WinFwPolicy::create(Socks5* proxy) {
  WinFwPolicy* fwPolicy = new WinFwPolicy(proxy);
  if (!fwPolicy->isValid()) {
    delete fwPolicy;
    return nullptr;
  }

  // If listening on a local TCP port, permit access only by web browsers.
  QTcpServer* tcpServer = qobject_cast<QTcpServer*>(proxy->parent());
  if ((tcpServer != nullptr) && tcpServer->serverAddress().isLoopback()) {
    fwPolicy->restrictProxyPort(tcpServer->serverPort());
  }

  return fwPolicy;
}

WinFwPolicy::WinFwPolicy(QObject* parent) : QObject(parent) {
  // Create the firewall engine handle
  FWPM_SESSION0 session;
  memset(&session, 0, sizeof(session));
  session.flags = FWPM_SESSION_FLAG_DYNAMIC;
  DWORD result = FwpmEngineOpen0(nullptr, RPC_C_AUTHN_WINNT, nullptr, &session,
                                 &m_fwEngineHandle);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Failed to open firewall engine:"
             << WinUtils::win32strerror(result);
    m_fwEngineHandle = nullptr;
    return;
  }

  auto subscribe = [](void* ctx, const FWPM_SUBLAYER_CHANGE0* change) {
    auto fw = reinterpret_cast<WinFwPolicy*>(ctx);
    fw->fwpmSublayerChanged(change->changeType, change->subLayerKey);
  };

  // Watch for changes to the firewall
  GUID fwguid = KILLSWITCH_FW_GUID;
  FWPM_SUBLAYER_ENUM_TEMPLATE0 fwmatch{.providerKey = &fwguid};
  FWPM_SUBLAYER_SUBSCRIPTION0 fwsub = {0};
  fwsub.enumTemplate = &fwmatch;
  fwsub.flags = FWPM_SUBSCRIPTION_FLAG_NOTIFY_ON_ADD;
  fwsub.sessionKey = session.sessionKey;
  result = FwpmSubLayerSubscribeChanges0(m_fwEngineHandle, &fwsub, subscribe,
                                         this, &m_fwChangeHandle);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Failed to create firewall subscription:"
             << WinUtils::win32strerror(result);
    FwpmEngineClose0(m_fwEngineHandle);
    m_fwEngineHandle = nullptr;
    return;
  }

  // If the sublayer already exists - immediately generate a notification.
  FWPM_SUBLAYER0* fwlayer;
  result = FwpmSubLayerGetByKey0(m_fwEngineHandle, &fwguid, &fwlayer);
  if (result == ERROR_SUCCESS) {
    fwpmSublayerChanged(FWPM_CHANGE_ADD, KILLSWITCH_FW_GUID);
    FwpmFreeMemory0((void**)&fwlayer);
  }
}

WinFwPolicy::~WinFwPolicy() {
  if (m_fwEngineHandle) {
    GUID localProxyGuid = LOCALPROXY_FW_GUID;
    FwpmSubLayerUnsubscribeChanges0(m_fwEngineHandle, m_fwChangeHandle);
    FwpmSubLayerDeleteByKey0(m_fwEngineHandle, &localProxyGuid);
    FwpmEngineClose0(m_fwEngineHandle);
  }
}

void WinFwPolicy::fwpmSublayerChanged(uint changeType,
                                      const QUuid& subLayerKey) {
  // Ignore everything except sublayer creation.
  if (changeType != FWPM_CHANGE_ADD) {
    return;
  }
  if (subLayerKey != KILLSWITCH_FW_GUID) {
    return;
  }

  // Get the AppID for the current executable;
  FWP_BYTE_BLOB* appID = NULL;
  WCHAR filePath[MAX_PATH];
  GetModuleFileNameW(nullptr, filePath, MAX_PATH);
  DWORD result = FwpmGetAppIdFromFileName0(filePath, &appID);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Firewall setup failed:" << WinUtils::win32strerror(result);
    return;
  }
  auto appGuard = qScopeGuard([appID]() { FwpmFreeMemory0((void**)&appID); });

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

  // Start a transaction so that the firewall changes can be made atomically.
  FwpmTransactionBegin0(m_fwEngineHandle, 0);
  auto txnGuard =
      qScopeGuard([this]() { FwpmTransactionAbort0(m_fwEngineHandle); });

  WCHAR descv4out[] = L"Permit outbound IPv4 traffic from proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
  filter.displayData.description = descv4out;
  result = FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
  if (result != ERROR_SUCCESS) {
    return;
  }

  WCHAR descv4in[] = L"Permit inbound IPv4 traffic to proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
  filter.displayData.description = descv4in;
  result = FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
  if (result != ERROR_SUCCESS) {
    return;
  }

  WCHAR descv6out[] = L"Permit outbound IPv6 traffic from proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
  filter.displayData.description = descv6out;
  FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
  if (result != ERROR_SUCCESS) {
    return;
  }

  WCHAR descv6in[] = L"Permit inbound IPv6 traffic to proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;
  filter.displayData.description = descv6in;
  FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
  if (result != ERROR_SUCCESS) {
    return;
  }

  // Commit the transaction
  if (FwpmTransactionCommit0(m_fwEngineHandle) == ERROR_SUCCESS) {
    txnGuard.dismiss();
  } else {
    qDebug() << "Firewall setup failed:" << WinUtils::win32strerror(result);
  }
}

void WinFwPolicy::restrictProxyPort(quint16 port) {
  // Start a transaction so that the firewall changes can be made atomically.
  FwpmTransactionBegin0(m_fwEngineHandle, 0);
  auto txnGuard =
      qScopeGuard([this]() { FwpmTransactionAbort0(m_fwEngineHandle); });

  // Check if the Layer Already Exists
  GUID proxyguid = LOCALPROXY_FW_GUID;
  FWPM_SUBLAYER0* sublayer = nullptr;
  DWORD result = FwpmSubLayerGetByKey0(m_fwEngineHandle, &proxyguid, &sublayer);
  if (result == ERROR_SUCCESS) {
    FwpmFreeMemory0((void**)&sublayer);
    sublayer = nullptr;
  } else {
    FWPM_SUBLAYER0 newlayer;
    memset(&newlayer, 0, sizeof(newlayer));
    newlayer.subLayerKey = LOCALPROXY_FW_GUID;
    newlayer.displayData.name = (PWSTR)L"MozillaVPN-Proxy-Sublayer";
    newlayer.displayData.description =
        (PWSTR)L"Restrict application access to the proxy";
    newlayer.weight = 0xFFFF;
    result = FwpmSubLayerAdd0(m_fwEngineHandle, &newlayer, nullptr);
    if (result != ERROR_SUCCESS) {
      qDebug() << "Firewall setup failed:" << WinUtils::win32strerror(result);
      return;
    }
  }

  // Block TCP traffic sent the localhost port.
  FWPM_FILTER_CONDITION0 conds[4];
  conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
  conds[0].matchType = FWP_MATCH_EQUAL;
  conds[0].conditionValue.type = FWP_UINT8;
  conds[0].conditionValue.uint8 = IPPROTO_TCP;

  conds[1].fieldKey = FWPM_CONDITION_LOCAL_INTERFACE_TYPE;
  conds[1].matchType = FWP_MATCH_EQUAL;
  conds[1].conditionValue.type = FWP_UINT32;
  conds[1].conditionValue.uint32 = IF_TYPE_SOFTWARE_LOOPBACK;

  conds[2].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
  conds[2].matchType = FWP_MATCH_EQUAL;
  conds[2].conditionValue.type = FWP_UINT16;
  conds[2].conditionValue.uint16 = port;

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = conds;
  filter.numFilterConditions = 3;
  filter.action.type = FWP_ACTION_BLOCK;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = 1;
  filter.subLayerKey = LOCALPROXY_FW_GUID;
  filter.flags = FWPM_FILTER_FLAG_NONE;

  WCHAR descv4block[] = L"Block local IPv4 connections to proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
  filter.displayData.name = descv4block;
  filter.displayData.description = descv4block;
  result = FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Firewall setup failed:" << WinUtils::win32strerror(result);
    return;
  }
  WCHAR descv6block[] = L"Block local IPv6 connections to proxy";
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
  filter.displayData.name = descv6block;
  filter.displayData.description = descv6block;
  result = FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
  if (result != ERROR_SUCCESS) {
    qDebug() << "Firewall setup failed:" << WinUtils::win32strerror(result);
    return;
  }

  // Now for the fun part - permit all browsers!
  filter.weight.uint8 = 15;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.numFilterConditions = 4;
  QSettings hklm("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\StartMenuInternet",
                 QSettings::Registry64Format);
  for (const QString& name : hklm.childGroups()) {
    hklm.beginGroup(name);
    auto hklmGuard = qScopeGuard([&]() { hklm.endGroup(); });
    QVariant value = hklm.value("shell/open/command/Default");
    if (!value.isValid()) {
      continue;
    }
    // Let's just pretend Internet Explorer doesn't exist.
    // It's bundled into all kinds of Windows internals, to the point where I
    // don't think we can trust it to be a real user browser anymore.
    if (name.startsWith("iexplore", Qt::CaseInsensitive)) {
      continue;
    }

    // Strip unnecessary quotations by removing an equal number of leading and
    // trailing quotation marks.
    QString command = value.toString();
    while ((command.front() == '"') && (command.back() == '"')) {
      command = command.mid(1, command.size() - 2);
    }
    qDebug() << "Permitting browser traffic for:" << name;

    // Build the final condition to match the application ID.
    FWP_BYTE_BLOB* appID = NULL;
    result = FwpmGetAppIdFromFileName0((PCWSTR)command.utf16(), &appID);
    if (result != ERROR_SUCCESS) {
      qDebug() << "Failed to get appid for:" << name;
      continue;
    }
    auto appGuard = qScopeGuard([&]() { FwpmFreeMemory0((void**)&appID); });
    conds[3].fieldKey = FWPM_CONDITION_ALE_APP_ID;
    conds[3].matchType = FWP_MATCH_EQUAL;
    conds[3].conditionValue.type = FWP_BYTE_BLOB_TYPE;
    conds[3].conditionValue.byteBlob = appID;

    QString descv4allow = QString("Permit IPv4 connections from %1").arg(name);
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    filter.displayData.name = (LPWSTR)descv4allow.utf16();
    filter.displayData.description = (LPWSTR)descv4allow.utf16();
    result = FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
    if (result != ERROR_SUCCESS) {
      return;
    }

    QString descv6allow = QString("Permit IPv4 connections from %1").arg(name);
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
    filter.displayData.name = (LPWSTR)descv6allow.utf16();
    filter.displayData.description = (LPWSTR)descv6allow.utf16();
    result = FwpmFilterAdd0(m_fwEngineHandle, &filter, nullptr, nullptr);
    if (result != ERROR_SUCCESS) {
      return;
    }
  }

  // Commit the transaction
  if (FwpmTransactionCommit0(m_fwEngineHandle) == ERROR_SUCCESS) {
    txnGuard.dismiss();
  } else {
    qDebug() << "Firewall setup failed:" << WinUtils::win32strerror(result);
  }
}
