/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "winutils.h"

#include <WS2tcpip.h>
#include <fwpmu.h>
#include <netioapi.h>
#include <windows.h>
#include <winsock2.h>

#include <QString>
#include <QUuid>

// Fixed GUID of the Wireguard NT driver.
constexpr const QUuid WIREGUARD_NT_GUID(0xf64063ab, 0xbfee, 0x4881, 0xbf, 0x79,
                                        0x36, 0x6e, 0x4c, 0xc7, 0xba, 0x75);

QString WinUtils::win32strerror(unsigned long code) {
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

quint64 WinUtils::getVpnLuid() {
  // Get the LUID of the wireguard interface, if it's up.
  NET_LUID luid;
  GUID vpnInterfaceGuid = WIREGUARD_NT_GUID;
  if (ConvertInterfaceGuidToLuid(&vpnInterfaceGuid, &luid) != NO_ERROR) {
    return 0;
  }
  return luid.Value;
}
