/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSNETWORKWATCHER_H
#define WINDOWSNETWORKWATCHER_H

#include "networkwatcherimpl.h"

#include <windows.h>
#include <wlanapi.h>

class WindowsNetworkWatcher final : public NetworkWatcherImpl {
 public:
  WindowsNetworkWatcher(QObject* parent);
  ~WindowsNetworkWatcher();

  void initialize() override;

  void start() override;
  void stop() override;

  private:
  static void wlanCallback(PWLAN_NOTIFICATION_DATA data, PVOID context);

  void processWlan(PWLAN_NOTIFICATION_DATA data);

 private:
  HANDLE m_wlanHandle = nullptr;

  bool m_active = false;
};

#endif  // WINDOWSNETWORKWATCHER_H
