/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscrashclient.h"
#include "../logger.h"

#include <QObject>
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <codecvt>

#include <crashreporter/crashreporterapp.h>
using namespace std;

namespace {
Logger logger("CrashClient", "WindowsCrashServerClient");
}

constexpr auto ARG = L"crashreporter";

DWORD RecoveryCallback(PVOID contextParam) {
  BOOL cancelled;
  ApplicationRecoveryInProgress(&cancelled);
  WindowsCrashClient* client = static_cast<WindowsCrashClient*>(contextParam);

  client->launchUploader();
  ApplicationRecoveryFinished(true);
  return 0;
}

WindowsCrashClient::WindowsCrashClient() {}

WindowsCrashClient::~WindowsCrashClient() {}

bool WindowsCrashClient::start(int argc, char* argv[]) {
  m_launchPath = argv[0];
  if (!SUCCEEDED(RegisterApplicationRecoveryCallback(RecoveryCallback, this,
                                                     30000, 0))) {
    logger.error() << "Failed to register recovery callback.";
    return false;
  }
  return true;
}

bool WindowsCrashClient::launchUploader() {
  wstringstream pathStr;
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  pathStr << converter.from_bytes(m_launchPath) << L" " << ARG;
  wstring path = pathStr.str();
  STARTUPINFO si = {};
  PROCESS_INFORMATION pi = {};
  si.cb = sizeof(si);

  if (!CreateProcess(nullptr, &path[0], nullptr, nullptr, false, 0, nullptr,
                     nullptr, &si, &pi)) {
    logger.error() << "Failed to launch uploader.  " << GetLastError();
    return false;
  }
  return true;
}
