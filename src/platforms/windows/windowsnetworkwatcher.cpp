/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "windowscommons.h"

#include <QScopeGuard>

#pragma comment(lib, "Wlanapi.lib")

namespace {
Logger logger(LOG_WINDOWS, "WindowsNetworkWatcher");
}

WindowsNetworkWatcher::WindowsNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(WindowsNetworkWatcher);
}

WindowsNetworkWatcher::~WindowsNetworkWatcher() {
  MVPN_COUNT_DTOR(WindowsNetworkWatcher);

  if (m_wlanHandle) {
    WlanCloseHandle(m_wlanHandle, nullptr);
  }
}

void WindowsNetworkWatcher::initialize() {
  logger.log() << "initialize";

  DWORD negotiatedVersion;
  if (WlanOpenHandle(2, nullptr, &negotiatedVersion, &m_wlanHandle) !=
      ERROR_SUCCESS) {
    WindowsCommons::windowsLog("Failed to open the WLAN handle");
    return;
  }

  DWORD prefNotifSource;
  if (WlanRegisterNotification(m_wlanHandle, WLAN_NOTIFICATION_SOURCE_MSM,
                               true /* ignore duplicates */, wlanCallback, this,
                               nullptr, &prefNotifSource) != ERROR_SUCCESS) {
    WindowsCommons::windowsLog("Failed to register a wlan callback");
    return;
  }

  logger.log() << "callback registered";
}

// static
void WindowsNetworkWatcher::wlanCallback(PWLAN_NOTIFICATION_DATA data,
                                         PVOID context) {
  logger.log() << "Callback";

  WindowsNetworkWatcher* that = static_cast<WindowsNetworkWatcher*>(context);
  Q_ASSERT(that);

  that->processWlan(data);
}

void WindowsNetworkWatcher::processWlan(PWLAN_NOTIFICATION_DATA data) {
  logger.log() << "Processing wlan data";

  if (!isActive()) {
    logger.log() << "The watcher is off";
    return;
  }

  if (data->NotificationSource != WLAN_NOTIFICATION_SOURCE_MSM) {
    logger.log() << "The wlan source is not MSM";
    return;
  }

  if (data->NotificationCode != wlan_notification_msm_connected) {
    logger.log() << "The wlan code is not MSM connected";
    return;
  }

  PWLAN_CONNECTION_ATTRIBUTES connectionInfo = nullptr;
  DWORD connectionInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
  WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

  DWORD result = WlanQueryInterface(
      m_wlanHandle, &data->InterfaceGuid, wlan_intf_opcode_current_connection,
      nullptr, &connectionInfoSize, (PVOID*)&connectionInfo, &opCode);
  if (result != ERROR_SUCCESS) {
    WindowsCommons::windowsLog("Failed to query the interface");
    return;
  }

  auto guard = qScopeGuard([&] { WlanFreeMemory(connectionInfo); });

  if (connectionInfo->wlanSecurityAttributes.dot11AuthAlgorithm !=
          DOT11_AUTH_ALGO_80211_OPEN &&
      connectionInfo->wlanSecurityAttributes.dot11CipherAlgorithm !=
          DOT11_CIPHER_ALGO_WEP &&
      connectionInfo->wlanSecurityAttributes.dot11CipherAlgorithm !=
          DOT11_CIPHER_ALGO_WEP40 &&
      connectionInfo->wlanSecurityAttributes.dot11CipherAlgorithm !=
          DOT11_CIPHER_ALGO_WEP104) {
    logger.log() << "The network is secure enought";
    return;
  }

  QString ssid;
  for (size_t i = 0;
       i < connectionInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength;
       ++i) {
    ssid.append(QString::asprintf(
        "%c",
        (char)connectionInfo->wlanAssociationAttributes.dot11Ssid.ucSSID[i]));
  }

  QString bssid;
  for (size_t i = 0;
       i < sizeof(connectionInfo->wlanAssociationAttributes.dot11Bssid); ++i) {
    if (i == 5) {
      bssid.append(QString::asprintf(
          "%.2X\n", connectionInfo->wlanAssociationAttributes.dot11Bssid[i]));
    } else {
      bssid.append(QString::asprintf(
          "%.2X-", connectionInfo->wlanAssociationAttributes.dot11Bssid[i]));
    }
  }

  logger.log() << "Unsecure network:" << ssid << "id:" << bssid;
  emit unsecuredNetwork(ssid, bssid);
}
