/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSFIREWALLHELPER_H
#define WINDOWSFIREWALLHELPER_H

#include <windows.h>
#include <QString>
#include <QObject>

class WindowsFirewallHelper final : public QObject {
 public:
  ~WindowsFirewallHelper();
  bool excludeApp(const QString& exePath);
  static WindowsFirewallHelper* instance();
  static int getVPNInterfaceNumber();
  private:
  WindowsFirewallHelper(QObject* parent);
    HANDLE m_sessionHandle;
    GUID m_layerID;
};


#endif  // WINDOWSFIREWALLHELPER_H
