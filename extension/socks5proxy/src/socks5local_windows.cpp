/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// psapi.h must be included after windows.h
// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#include <QLocalSocket>
#include <QScopeGuard>

#include "socks5connection.h"

// static
QString Socks5Connection::localClientName(QLocalSocket* s) {
  // Get the process at the other end of the socket.
  HANDLE pipe = reinterpret_cast<HANDLE>(s->socketDescriptor());
  ULONG pid;
  if (!GetNamedPipeClientProcessId(pipe, &pid)) {
    return QString();
  }
  HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
  if (handle == nullptr) {
    return QString();
  }
  auto guard = qScopeGuard([handle]() { CloseHandle(handle); });

  // Read the process filename.
  WCHAR filename[MAX_PATH];
  DWORD len = GetProcessImageFileNameW(handle, filename, MAX_PATH);
  if (len > 0) {
    return QString::fromWCharArray(filename, len);
  }
  return QString();
}
