/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCOMMONS_H
#define WINDOWSCOMMONS_H

#include <QString>
#include <Windows.h>

class WindowsCommons final {
 public:
  static QString getErrorMessage();
  static void windowsLog(const QString& msg);
  static void windowsLog(const QString& msg, DWORD err);

  static QString tunnelConfigFile();
  static QString tunnelLogFile();

  // Returns the Interface Index of the VPN Adapter
  static int VPNAdapterIndex();
  // Returns the Interface Index that could Route to 1.1.1.1
  static int CurrentGatewayIndex();
};

#endif  // WINDOWSCOMMONS_H
