/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "server.h"

#include <Windows.h>
#include <winsvc.h>

constexpr const DWORD MODULE_FILENAME_MAX = 32768;

bool WebExtension::Server::isAllowedToConnect(qintptr sd) {
  wchar_t clientModuleName[MODULE_FILENAME_MAX];
  wchar_t selfModuleName[MODULE_FILENAME_MAX];
  
  // Get the filename of the current process.
  DWORD len = ::GetModuleFileNameW(nullptr, selfModuleName, MODULE_FILENAME_MAX);
  if (!len || (len > MODULE_FILENAME_MAX)) {
    return false;
  }

  // Get a handle to the client process at the other end of the named pipe.
  ULONG clientPid = 0;
  if (!GetNamedPipeClientProcessId(reinterpret_cast<HANDLE>(sd), &clientPid)) {
    return false;
  }
  if (clientPid == 0) {
    return false;
  }

  HANDLE clientProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                  clientPid);
  if (!clientProc) {
    return false;
  }
  auto procGuard = qScopeGuard([&] { ::CloseHandle(clientProc); });

  // The client process should have the same module name.
  len = MODULE_FILENAME_MAX;
  if (!QueryFullProcessImageNameW(clientProc, 0, clientModuleName, &len)) {
    return false;
  }
  return wcscmp(selfModuleName, clientModuleName) == 0;
}
