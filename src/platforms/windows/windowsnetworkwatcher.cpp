/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "windowscommons.h"

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

  logger.log() << "all good";
}

void WindowsNetworkWatcher::start() {
  logger.log() << "actived";
  m_active = true;
}

void WindowsNetworkWatcher::stop() {
  logger.log() << "deactived";
  m_active = false;
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

  if (!m_active) {
    return;
  }

  // TODO
}
