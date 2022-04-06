/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscrashclient.h"

#include "crashreporter/crashreporterapp.h"
#include "../logger.h"

#include <codecvt>
#include <iostream>
#include <QDir>
#include <QObject>
#include <QStandardPaths>
#include <sstream>
#include <Windows.h>

using namespace std;

namespace {
Logger logger("CrashClient", "WindowsCrashServerClient");
}

constexpr auto ARG = L"crashreporter";

DWORD RecoveryCallback(PVOID contextParam) {
  logger.debug() << "Notified of application crash.";
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
  logger.debug() << "Starting crash client.";
  auto appDatas =
      QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
  auto appLocal = appDatas.first() + "\\dumps";
  if (!QDir().mkpath(appLocal)) {
    logger.error() << "Unable to create dump folder at " << appLocal;
  }

  m_launchPath = argv[0];
  if (!SUCCEEDED(RegisterApplicationRecoveryCallback(RecoveryCallback, this,
                                                     30000, 0))) {
    logger.error() << "Failed to register recovery callback.";
    return false;
  }
  return true;
}

bool WindowsCrashClient::launchUploader() {
  logger.debug() << "Trying to launch crash reporter.";
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
