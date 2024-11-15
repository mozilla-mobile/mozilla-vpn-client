/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "winutils.h"

#include <windows.h>

#include <QString>

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
