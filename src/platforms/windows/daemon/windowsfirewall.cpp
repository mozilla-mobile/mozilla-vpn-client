/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsfirewall.h"
#include "logger.h"
#include "leakdetector.h"
#include "../windowscommons.h"
#include "../../daemon/interfaceconfig.h"
#include "../../ipaddressrange.h"
#include "../../ipaddress.h"

#include <QApplication>
#include <QObject>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QScopeGuard>
#include <QHostAddress>
#include <QtEndian>
#include <windows.h>
#include <fwpmu.h>
#include <stdio.h>
#include <comdef.h>
#include <netfw.h>
#include "winsock.h"

#include <initguid.h>
#include <guiddef.h>
#include <qaccessible.h>

#define IPV6_ADDRESS_SIZE 16

// ID for the Firewall Sublayer
DEFINE_GUID(ST_FW_WINFW_BASELINE_SUBLAYER_KEY, 0xc78056ff, 0x2bc1, 0x4211, 0xaa,
            0xdd, 0x7f, 0x35, 0x8d, 0xef, 0x20, 0x2d);
// ID for the Mullvad Split-Tunnel Sublayer Provider
DEFINE_GUID(ST_FW_PROVIDER_KEY, 0xe2c114ee, 0xf32a, 0x4264, 0xa6, 0xcb, 0x3f,
            0xa7, 0x99, 0x63, 0x56, 0xd9);

namespace {
Logger logger(LOG_WINDOWS, "WindowsFirewall");
WindowsFirewall* s_instance = nullptr;

// Note Filter Weight may be between 0-15!
constexpr uint8_t LOW_WEIGHT = 0;
constexpr uint8_t MED_WEIGHT = 7;
constexpr uint8_t HIGH_WEIGHT = 13;
constexpr uint8_t MAX_WEIGHT = 15;
}  // namespace

WindowsFirewall* WindowsFirewall::instance() {
  if (s_instance == nullptr) {
    s_instance = new WindowsFirewall(qApp);
  }
  return s_instance;
}

WindowsFirewall::WindowsFirewall(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(WindowsFirewall);
  Q_ASSERT(s_instance == nullptr);

  HANDLE engineHandle = NULL;
  DWORD result = ERROR_SUCCESS;
  // Use dynamic sessions for efficiency and safety:
  //  -> Filtering policy objects are deleted even when the application crashes/
  //  deamon goes down
  FWPM_SESSION0 session;
  memset(&session, 0, sizeof(session));
  session.flags = FWPM_SESSION_FLAG_DYNAMIC;

  logger.debug() << "Opening the filter engine.";

  result =
      FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &engineHandle);

  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog("FwpmEngineOpen0 failed");
    return;
  }
  logger.debug() << "Filter engine opened successfully.";
  m_sessionHandle = engineHandle;
}

WindowsFirewall::~WindowsFirewall() {
  MVPN_COUNT_DTOR(WindowsFirewall);
  if (m_sessionHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(m_sessionHandle);
  }
}

bool WindowsFirewall::init() {
  if (m_init) {
    logger.warning() << "Alread initialised FW_WFP layer";
    return true;
  }
  if (m_sessionHandle == INVALID_HANDLE_VALUE) {
    logger.error() << "Cant Init Sublayer with invalid wfp handle";
    return false;
  }
  // If we were not able to aquire a handle, this will fail anyway.
  // We need to open up another handle because of wfp rules:
  // If a wfp resource was created with SESSION_DYNAMIC,
  // the session exlusively owns the resource, meaning the driver can't add
  // filters to the sublayer. So let's have non dynamic session only for the
  // sublayer creation. This means the Layer exists until the next Reboot.
  DWORD result = ERROR_SUCCESS;
  HANDLE wfp = INVALID_HANDLE_VALUE;
  FWPM_SESSION0 session;
  memset(&session, 0, sizeof(session));

  logger.debug() << "Opening the filter engine";
  result = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &wfp);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmEngineOpen0 failed. Return value:.\n" << result;
    return false;
  }
  auto cleanup = qScopeGuard([&] { FwpmEngineClose0(wfp); });

  // Check if the Layer Already Exists
  FWPM_SUBLAYER0* maybeLayer;
  result = FwpmSubLayerGetByKey0(wfp, &ST_FW_WINFW_BASELINE_SUBLAYER_KEY,
                                 &maybeLayer);
  if (result == ERROR_SUCCESS) {
    logger.debug() << "The Sublayer Already Exists!";
    FwpmFreeMemory0((void**)&maybeLayer);
    return true;
  }

  // Step 1: Start Transaction
  result = FwpmTransactionBegin(wfp, NULL);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }

  // Step 3: Add Sublayer
  FWPM_SUBLAYER0 subLayer;
  memset(&subLayer, 0, sizeof(subLayer));
  subLayer.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;
  subLayer.displayData.name = (PWSTR)L"MozillaVPN-SplitTunnel-Sublayer";
  subLayer.displayData.description =
      (PWSTR)L"Filters that enforce a good baseline";
  subLayer.weight = 0xFFFF;

  result = FwpmSubLayerAdd0(wfp, &subLayer, NULL);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmSubLayerAdd0 failed. Return value:.\n" << result;
    return false;
  }
  // Step 4: Commit!
  result = FwpmTransactionCommit0(wfp);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  logger.debug() << "Initialised Sublayer";
  m_init = true;
  return true;
}

bool WindowsFirewall::allowTrafficForAppOnAdapter(const QString& exePath,
                                                  int network) {
  logger.debug() << "Adding hard Permit for " << exePath
                 << " Accessing Device: " << network;
  DWORD result = ERROR_SUCCESS;

  // Get the AppID for the Executable;
  QString appName = QFileInfo(exePath).baseName();
  std::wstring wstr = exePath.toStdWString();
  PCWSTR appPath = wstr.c_str();
  FWP_BYTE_BLOB* appID = NULL;
  uint64_t filterID = 0;

  result = FwpmGetAppIdFromFileName0(appPath, &appID);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog("FwpmGetAppIdFromFileName0 failure");
    return false;
  }
  // Condition 1: Request must come from the .exe
  FWPM_FILTER_CONDITION0 conds[2];
  conds[0].fieldKey = FWPM_CONDITION_ALE_APP_ID;
  conds[0].matchType = FWP_MATCH_EQUAL;
  conds[0].conditionValue.type = FWP_BYTE_BLOB_TYPE;
  conds[0].conditionValue.byteBlob = appID;

  // Condition 2: Request may NOT be targeting the TUN interface
  conds[1].fieldKey = FWPM_CONDITION_INTERFACE_INDEX;
  conds[1].matchType = FWP_MATCH_EQUAL;
  conds[1].conditionValue.type = FWP_UINT32;
  conds[1].conditionValue.uint32 = network;

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = conds;
  filter.numFilterConditions = 2;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = 15;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;
  filter.flags = FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT;  // Make this decision
                                                       // only blockable by veto

  // Build and add the Filters
  // #1 Permit outbound IPv4 traffic.
  {
    QString name = QString("Permit (out) IPv4 Traffic of: " + appName);
    std::wstring wname = name.toStdWString();
    PCWSTR filterName = wname.c_str();
    filter.displayData.name = (PWSTR)filterName;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);

    if (result != ERROR_SUCCESS) {
      logger.error() << "Failed to set rule " << name << "\n" << result;
      return false;
    }
  }
  // #2 Permit inbound IPv4 traffic.
  {
    QString name = QString("Permit (in)  IPv4 Traffic of: " + appName);
    std::wstring wname = name.toStdWString();
    PCWSTR filterName = wname.c_str();
    filter.displayData.name = (PWSTR)filterName;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);

    if (result != ERROR_SUCCESS) {
      logger.error() << "Failed to set rule " << name << "\n" << result;
      return false;
    }
  }
  // #3 Permit outbound IPv6 traffic.
  {
    QString name = QString("Permit (out) IPv6 Traffic of: " + appName);
    std::wstring wname = name.toStdWString();
    PCWSTR filterName = wname.c_str();
    filter.displayData.name = (PWSTR)filterName;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);

    if (result != ERROR_SUCCESS) {
      logger.error() << "Failed to set rule " << name << "\n" << result;
      return false;
    }
  }
  // #4 Permit inbound IPv6 traffic.
  {
    QString name = QString("Permit (in)  IPv6 Traffic of: " + appName);
    std::wstring wname = name.toStdWString();
    PCWSTR filterName = wname.c_str();
    filter.displayData.name = (PWSTR)filterName;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;
    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);

    if (result != ERROR_SUCCESS) {
      logger.error() << "Failed to set rule " << name << "\n" << result;
      return false;
    }
  }

  logger.debug() << "Ruleset applied for: " << exePath;
  return true;
}

bool WindowsFirewall::enableKillSwitch(int vpnAdapterIndex,
                                       const InterfaceConfig& config) {
// Checks if the FW_Rule was enabled succesfully,
// disables the whole killswitch and returns false if not.
#define FW_OK(rule, name)                             \
  if (!rule) {                                        \
    logger.error() << "Rule failed:" << name << "\n"; \
    disableKillSwitch();                              \
    return false;                                     \
  }                                                   \
  logger.debug() << "Rule enabled:" << name << "\n";

  logger.info() << "Enabling Killswitch Using Adapter:" << vpnAdapterIndex;
  FW_OK(blockTrafficTo(config.m_allowedIPAddressRanges, LOW_WEIGHT),
        "Block all");
  FW_OK(allowTrafficOfAdapter(vpnAdapterIndex, MED_WEIGHT),
        "Allow Traffic to VPN Adapter");
  FW_OK(allowDHCPTraffic(MED_WEIGHT), "Allow DHCP Traffic");
  FW_OK(allowHyperVTraffic(MED_WEIGHT), "Allow Hyper-V Traffic");
  FW_OK(allowTrafficForAppOnAll(getCurrentPath(), MAX_WEIGHT),
        "Allow Traffic for MozillaVPN.exe");
  FW_OK(blockTrafficOnPort(53, MED_WEIGHT), "Block DNS Traffic");
  FW_OK(allowTrafficTo(QHostAddress(config.m_dnsServer), 53, HIGH_WEIGHT),
        "Allow DNS Traffic to Defined DNS Server");

  logger.debug() << "Killswitch on! Rules:" << m_activeRules.length();
  return true;
#undef FW_OK
}

bool WindowsFirewall::disableKillSwitch() {
  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }

  for (const auto& filterID : qAsConst(m_activeRules)) {
    FwpmFilterDeleteById0(m_sessionHandle, filterID);
  }

  // Commit!
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  m_activeRules.clear();
  logger.debug() << "Firewall Disabled!";
  return true;
}

bool WindowsFirewall::allowTrafficForAppOnAll(const QString& exePath,
                                              int weight) {
  logger.debug() << "Adding hard Permit for " << exePath
                 << " Accessing All Devices: ";
  DWORD result = ERROR_SUCCESS;
  Q_ASSERT(weight <= 15);

  // Get the AppID for the Executable;
  QString appName = QFileInfo(exePath).baseName();
  std::wstring wstr = exePath.toStdWString();
  PCWSTR appPath = wstr.c_str();
  FWP_BYTE_BLOB* appID = NULL;
  uint64_t filterID = 0;
  result = FwpmGetAppIdFromFileName0(appPath, &appID);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog("FwpmGetAppIdFromFileName0 failure");
    return false;
  }

  // Start Transaction
  result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }

  // Condition: Request must come from the .exe
  FWPM_FILTER_CONDITION0 conds;
  conds.fieldKey = FWPM_CONDITION_ALE_APP_ID;
  conds.matchType = FWP_MATCH_EQUAL;
  conds.conditionValue.type = FWP_BYTE_BLOB_TYPE;
  conds.conditionValue.byteBlob = appID;

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = &conds;
  filter.numFilterConditions = 1;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = weight;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;
  filter.flags = FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT;  // Make this decision
                                                       // only blockable by veto

  // Build and add the Filters
  // #1 Permit outbound IPv4 traffic.
  {
    QString name = QString("Permit (out) IPv4 Traffic of: " + appName);
    std::wstring wname = name.toStdWString();
    PCWSTR filterName = wname.c_str();
    filter.displayData.name = (PWSTR)filterName;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);

    if (result != ERROR_SUCCESS) {
      logger.error() << "Failed to set rule " << name << "\n" << result;
      return false;
    }
  }
  // #2 Permit inbound IPv4 traffic.
  {
    QString name = QString("Permit (in)  IPv4 Traffic of: " + appName);
    std::wstring wname = name.toStdWString();
    PCWSTR filterName = wname.c_str();
    filter.displayData.name = (PWSTR)filterName;
    filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);

    if (result != ERROR_SUCCESS) {
      logger.error() << "Failed to set rule " << name << "\n" << result;
      return false;
    }
  }

  // Commit!
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}

bool WindowsFirewall::allowTrafficOfAdapter(int networkAdapter,
                                            uint8_t weight) {
  // Start Transaction
  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }

  FWPM_FILTER_CONDITION0 conds;
  // Condition: Request must be targeting the TUN interface
  conds.fieldKey = FWPM_CONDITION_INTERFACE_INDEX;
  conds.matchType = FWP_MATCH_EQUAL;
  conds.conditionValue.type = FWP_UINT32;
  conds.conditionValue.uint32 = networkAdapter;

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = &conds;
  filter.numFilterConditions = 1;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = weight;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

  uint64_t filterID = 0;
  // #1 Permit outbound IPv4 traffic.
  std::wstring name(L"Permit outbound IPv4 traffic on TUN adapter");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog(
        "Failed: Permit outbound IPv4 traffic on adapter");
    logger.debug() << "Result code " << result;
    return false;
  }
  m_activeRules.append(filterID);

  // #2 Permit inbound IPv4 traffic.
  name = std::wstring(L"Permit inbound IPv4 traffic on TUN adapter");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog(
        "Failed: Permit inbound IPv4 traffic on adapter");
    logger.debug() << "Result code " << result;
    return false;
  }
  m_activeRules.append(filterID);

  // #3 Permit outbound IPv6 traffic.
  name = std::wstring(L"Permit outbound IPv6 traffic on TUN adapter");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog(
        "Failed: Permit outbound IPv6 traffic on adapter");
    logger.debug() << "Result code " << result;
    return false;
  }
  m_activeRules.append(filterID);

  // #4 Permit inbound IPv6 traffic.
  name = std::wstring(L"Permit inbound IPv4 traffic on TUN adapter");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog(
        "Failed: Permit inbound IPv6 traffic on adapter");
    return false;
  }
  m_activeRules.append(filterID);

  // Commit!
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}

bool WindowsFirewall::allowTrafficTo(const QHostAddress& targetIP, uint port,
                                     int weight) {
  logger.debug() << "Requesting to allow Traffic to: " << targetIP.toString()
                 << ":" << port;

  bool isIPv4 = targetIP.protocol() == QAbstractSocket::IPv4Protocol;
  GUID layerOut =
      isIPv4 ? FWPM_LAYER_ALE_AUTH_CONNECT_V4 : FWPM_LAYER_ALE_AUTH_CONNECT_V6;
  GUID layoutIn = isIPv4 ? FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4
                         : FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;

  quint32_be ipBigEndian;
  quint32 ip = targetIP.toIPv4Address();
  qToBigEndian(ip, &ipBigEndian);

  // Start Transaction
  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }
  // Allow Traffic to IP with PORT using any protocol
  FWPM_FILTER_CONDITION0 conds[4];
  conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
  conds[0].matchType = FWP_MATCH_EQUAL;
  conds[0].conditionValue.type = FWP_UINT8;
  conds[0].conditionValue.uint8 = (IPPROTO_UDP);

  conds[1].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
  conds[1].matchType = FWP_MATCH_EQUAL;
  conds[1].conditionValue.type = FWP_UINT8;
  conds[1].conditionValue.uint16 = (IPPROTO_TCP);

  conds[2].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
  conds[2].matchType = FWP_MATCH_EQUAL;
  conds[2].conditionValue.type = FWP_UINT16;
  conds[2].conditionValue.uint16 = port;

  conds[3].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
  conds[3].matchType = FWP_MATCH_EQUAL;
  importAddress(targetIP, conds[3].conditionValue);

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = conds;
  filter.numFilterConditions = 4;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = weight;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;
  filter.flags = FWPM_FILTER_FLAG_CLEAR_ACTION_RIGHT;  // Hard Permit!

  uint64_t filterID = 0;
  auto name = std::wstring(L"Permit outbound Traffic to Fixed IP");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = layerOut;
  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    return false;
  }
  m_activeRules.append(filterID);

  name = std::wstring(L"Permit inbound Traffic from Fixed IP");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = layoutIn;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    return false;
  }
  m_activeRules.append(filterID);
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}

bool WindowsFirewall::allowDHCPTraffic(uint8_t weight) {
  // Start Transaction
  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }
  // Allow outbound DHCPv4
  {
    FWPM_FILTER_CONDITION0 conds[4];
    // Condition: Request must be targeting the TUN interface
    conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
    conds[0].matchType = FWP_MATCH_EQUAL;
    conds[0].conditionValue.type = FWP_UINT8;
    conds[0].conditionValue.uint8 = (IPPROTO_UDP);

    conds[1].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
    conds[1].matchType = FWP_MATCH_EQUAL;
    conds[1].conditionValue.type = FWP_UINT16;
    conds[1].conditionValue.uint16 = (68);

    conds[2].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
    conds[2].matchType = FWP_MATCH_EQUAL;
    conds[2].conditionValue.type = FWP_UINT16;
    conds[2].conditionValue.uint16 = 67;

    conds[3].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
    conds[3].matchType = FWP_MATCH_EQUAL;
    conds[3].conditionValue.type = FWP_UINT32;
    conds[3].conditionValue.uint32 = (0xffffffff);

    // Assemble the Filter base
    FWPM_FILTER0 filter;
    memset(&filter, 0, sizeof(filter));
    filter.filterCondition = conds;
    filter.numFilterConditions = 4;
    filter.action.type = FWP_ACTION_PERMIT;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = weight;
    filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

    uint64_t filterID = 0;
    auto name = std::wstring(L"Permit outbound DHCPv4");
    filter.displayData.name = (PWSTR)name.c_str();
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
    if (result != ERROR_SUCCESS) {
      return false;
    }
    m_activeRules.append(filterID);
  }
  // Allow inbound DHCPv4
  {
    FWPM_FILTER_CONDITION0 conds[3];
    conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
    conds[0].matchType = FWP_MATCH_EQUAL;
    conds[0].conditionValue.type = FWP_UINT8;
    conds[0].conditionValue.uint8 = (IPPROTO_UDP);

    conds[1].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
    conds[1].matchType = FWP_MATCH_EQUAL;
    conds[1].conditionValue.type = FWP_UINT16;
    conds[1].conditionValue.uint16 = (68);

    conds[2].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
    conds[2].matchType = FWP_MATCH_EQUAL;
    conds[2].conditionValue.type = FWP_UINT16;
    conds[2].conditionValue.uint16 = 67;

    // Assemble the Filter base
    FWPM_FILTER0 filter;
    memset(&filter, 0, sizeof(filter));
    filter.filterCondition = conds;
    filter.numFilterConditions = 3;
    filter.action.type = FWP_ACTION_PERMIT;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = weight;
    filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

    uint64_t filterID = 0;
    auto name = std::wstring(L"Permit inbound DHCPv4");
    filter.displayData.name = (PWSTR)name.c_str();
    filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;

    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
    if (result != ERROR_SUCCESS) {
      return false;
    }
    m_activeRules.append(filterID);
  }

  // Allow outbound DHCPv6
  {
    FWPM_FILTER_CONDITION0 conds[3];
    // Condition: Request must be targeting the TUN interface
    conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
    conds[0].matchType = FWP_MATCH_EQUAL;
    conds[0].conditionValue.type = FWP_UINT8;
    conds[0].conditionValue.uint8 = (IPPROTO_UDP);

    conds[1].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
    conds[1].matchType = FWP_MATCH_EQUAL;
    conds[1].conditionValue.type = FWP_UINT16;
    conds[1].conditionValue.uint16 = (68);

    conds[2].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
    conds[2].matchType = FWP_MATCH_EQUAL;
    conds[2].conditionValue.type = FWP_UINT16;
    conds[2].conditionValue.uint16 = 67;

    // Assemble the Filter base
    FWPM_FILTER0 filter;
    memset(&filter, 0, sizeof(filter));
    filter.filterCondition = conds;
    filter.numFilterConditions = 3;
    filter.action.type = FWP_ACTION_PERMIT;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = weight;
    filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

    uint64_t filterID = 0;
    auto name = std::wstring(L"Permit outbound DHCPv6");
    filter.displayData.name = (PWSTR)name.c_str();
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;

    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
    if (result != ERROR_SUCCESS) {
      return false;
    }
    m_activeRules.append(filterID);
  }

  // Allow inbound DHCPv6
  {
    FWPM_FILTER_CONDITION0 conds[3];
    conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
    conds[0].matchType = FWP_MATCH_EQUAL;
    conds[0].conditionValue.type = FWP_UINT8;
    conds[0].conditionValue.uint8 = (IPPROTO_UDP);

    conds[1].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
    conds[1].matchType = FWP_MATCH_EQUAL;
    conds[1].conditionValue.type = FWP_UINT16;
    conds[1].conditionValue.uint16 = (68);

    conds[2].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
    conds[2].matchType = FWP_MATCH_EQUAL;
    conds[2].conditionValue.type = FWP_UINT16;
    conds[2].conditionValue.uint16 = 67;

    // Assemble the Filter base
    FWPM_FILTER0 filter;
    memset(&filter, 0, sizeof(filter));
    filter.filterCondition = conds;
    filter.numFilterConditions = 3;
    filter.action.type = FWP_ACTION_PERMIT;
    filter.weight.type = FWP_UINT8;
    filter.weight.uint8 = weight;
    filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

    uint64_t filterID = 0;
    auto name = std::wstring(L"Permit inbound DHCPv6");
    filter.displayData.name = (PWSTR)name.c_str();
    filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;

    result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
    if (result != ERROR_SUCCESS) {
      return false;
    }
    m_activeRules.append(filterID);
  }

  // Commit!
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}
// Allows the internal Hyper-V Switches to work.
bool WindowsFirewall::allowHyperVTraffic(uint8_t weight) {
  // Start Transaction
  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }

  FWPM_FILTER_CONDITION0 cond;
  // Condition: Request must be targeting the TUN interface
  cond.fieldKey = FWPM_CONDITION_L2_FLAGS;
  cond.matchType = FWP_MATCH_EQUAL;
  cond.conditionValue.type = FWP_UINT32;
  cond.conditionValue.uint32 = FWP_CONDITION_L2_IS_VM2VM;

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.filterCondition = &cond;
  filter.numFilterConditions = 1;
  filter.action.type = FWP_ACTION_PERMIT;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = weight;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

  uint64_t filterID = 0;
  // #1 Permit Hyper-V => Hyper-V outbound.
  std::wstring name(L"Permit Hyper-V => Hyper-V outbound");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_OUTBOUND_MAC_FRAME_NATIVE;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    logger.error() << "Permit Hyper-V => Hyper-V outbound failed " << result;
    return false;
  }
  m_activeRules.append(filterID);
  // #2 Permit Hyper-V => Hyper-V inbound.
  name = std::wstring(L"Permit Hyper-V => Hyper-V inbound");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_INBOUND_MAC_FRAME_NATIVE;

  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    logger.debug() << "Permit Hyper-V => Hyper-V inbound failed " << result;
    return false;
  }
  m_activeRules.append(filterID);

  // Commit!
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}

bool WindowsFirewall::blockTrafficTo(const IPAddressRange& range,
                                     uint8_t weight) {
  logger.info() << "Blocking traffic to " << range.toString();
  IPAddress addr = IPAddress::create(range.toString());

  auto lower = addr.address();
  auto upper = addr.broadcastAddress();

  // Start Transaction
  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }
  const bool isV4 = addr.type() == QAbstractSocket::IPv4Protocol;
  const GUID layerKeyOut =
      isV4 ? FWPM_LAYER_ALE_AUTH_CONNECT_V4 : FWPM_LAYER_ALE_AUTH_CONNECT_V6;
  const GUID layerKeyIn = isV4 ? FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4
                               : FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;

  uint64_t filterID = 0;

  // Assemble the Filter base
  FWPM_FILTER0 filter;
  memset(&filter, 0, sizeof(filter));
  filter.action.type = FWP_ACTION_BLOCK;
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = weight;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

  FWPM_FILTER_CONDITION0 cond[1] = {0};
  FWP_RANGE0 ipRange;
  importAddress(lower, ipRange.valueLow);
  importAddress(upper, ipRange.valueHigh);

  cond[0].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
  cond[0].matchType = FWP_MATCH_RANGE;
  cond[0].conditionValue.type = FWP_RANGE_TYPE;
  cond[0].conditionValue.rangeValue = &ipRange;

  filter.numFilterConditions = 1;
  filter.filterCondition = cond;

  {  // Set outbound rule for iprange
    QString name =
        QString("Block Outbound traffic to %1 ").arg(range.toString());
    std::wstring wname = name.toStdWString();
    filter.displayData.name = (PWSTR)wname.c_str();
    filter.layerKey = layerKeyOut;

    if (result != ERROR_SUCCESS) {
      return false;
    }
    m_activeRules.append(filterID);
  }
  {  // Set inbound rule for iprange
    QString name =
        QString("Block Inbound traffic to %1 ").arg(range.toString());
    std::wstring wname = name.toStdWString();
    filter.displayData.name = (PWSTR)wname.c_str();
    filter.layerKey = layerKeyIn;

    if (result != ERROR_SUCCESS) {
      return false;
    }
    m_activeRules.append(filterID);
  }

  // Commit!
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}

bool WindowsFirewall::blockTrafficTo(const QList<IPAddressRange>& rangeList,
                                     uint8_t weight) {
  for (auto range : rangeList) {
    if (!blockTrafficTo(range, weight)) {
      logger.info() << "Setting Range of" << range.toString() << "failed";
      return false;
    }
  }
  return true;
}

// Returns the Path of the Current Executable this runs in
QString WindowsFirewall::getCurrentPath() {
  const unsigned char initValue = 0xff;
  QByteArray buffer(2048, initValue);
  auto ok = GetModuleFileNameA(NULL, buffer.data(), buffer.size());

  if (ok == ERROR_INSUFFICIENT_BUFFER) {
    buffer.resize(buffer.size() * 2);
    ok = GetModuleFileNameA(NULL, buffer.data(), buffer.size());
  }
  if (ok == 0) {
    WindowsCommons::windowsLog("Err fetching dos path");
    return "";
  }

  return QString::fromLocal8Bit(buffer);
}

void WindowsFirewall::importAddress(const QHostAddress& addr,
                                    OUT FWP_VALUE0_& value) {
  const bool isV4 = addr.protocol() == QAbstractSocket::IPv4Protocol;
  if (isV4) {
    value.type = FWP_UINT32;
    value.uint32 = addr.toIPv4Address();
    return;
  }
  value.type = FWP_BYTE_ARRAY16_TYPE;
  auto v6bytes = addr.toIPv6Address();
  RtlCopyMemory(&v6bytes, value.byteArray16, IPV6_ADDRESS_SIZE);
}
void WindowsFirewall::importAddress(const QHostAddress& addr,
                                    OUT FWP_CONDITION_VALUE0_& value) {
  const bool isV4 = addr.protocol() == QAbstractSocket::IPv4Protocol;
  if (isV4) {
    value.type = FWP_UINT32;
    value.uint32 = addr.toIPv4Address();
    return;
  }
  value.type = FWP_BYTE_ARRAY16_TYPE;
  auto v6bytes = addr.toIPv6Address();
  RtlCopyMemory(&v6bytes, value.byteArray16, IPV6_ADDRESS_SIZE);
}

bool WindowsFirewall::blockTrafficOnPort(uint port, uint8_t weight) {
  logger.debug() << "Requesting to block all traffic on port " << port;

  auto result = FwpmTransactionBegin(m_sessionHandle, NULL);
  auto cleanup = qScopeGuard([&] {
    if (result != ERROR_SUCCESS) {
      FwpmTransactionAbort0(m_sessionHandle);
    }
  });
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionBegin0 failed. Return value:.\n"
                   << result;
    return false;
  }
  // Allow Traffic to IP with PORT using any protocol
  FWPM_FILTER_CONDITION0 conds[3];
  conds[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
  conds[0].matchType = FWP_MATCH_EQUAL;
  conds[0].conditionValue.type = FWP_UINT8;
  conds[0].conditionValue.uint8 = (IPPROTO_UDP);

  conds[1].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
  conds[1].matchType = FWP_MATCH_EQUAL;
  conds[1].conditionValue.type = FWP_UINT8;
  conds[1].conditionValue.uint8 = (IPPROTO_TCP);

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
  filter.weight.uint8 = weight;
  filter.subLayerKey = ST_FW_WINFW_BASELINE_SUBLAYER_KEY;

  uint64_t filterID = 0;
  auto name = std::wstring(L"Block Traffic on Port");
  filter.displayData.name = (PWSTR)name.c_str();
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    return false;
  }
  m_activeRules.append(filterID);
  filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    return false;
  }
  m_activeRules.append(filterID);
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    return false;
  }
  m_activeRules.append(filterID);
  filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6;
  result = FwpmFilterAdd0(m_sessionHandle, &filter, NULL, &filterID);
  if (result != ERROR_SUCCESS) {
    return false;
  }
  m_activeRules.append(filterID);
  result = FwpmTransactionCommit0(m_sessionHandle);
  if (result != ERROR_SUCCESS) {
    logger.error() << "FwpmTransactionCommit0 failed. Return value:.\n"
                   << result;
    return false;
  }
  return true;
}
