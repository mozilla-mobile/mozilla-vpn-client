/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSFIREWALLHELPER_H
#define WINDOWSFIREWALLHELPER_H

#include <QObject>
#include <QQuickImageProvider>
#include <windows.h>
#include <fwpmu.h>


class WindowsFirewallHelper final {
 public:
  ~WindowsFirewallHelper();
  bool excludeApp(QString exePath);
  static WindowsFirewallHelper* instance();
  private:
    WindowsFirewallHelper();
    HANDLE mSessionHandle;
    GUID mLayerID;
};


#endif  // WINDOWSFIREWALLHELPER_H
