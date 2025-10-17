/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webexthandler.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QString>

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

static DWORD getParentProcessId(DWORD pid) {
  HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (h == INVALID_HANDLE_VALUE) {
    return 0;
  }
  auto guard = qScopeGuard([h]() { CloseHandle(h); });
  
  PROCESSENTRY32W entry = {.dwSize = sizeof(PROCESSENTRY32W)};
  if (!Process32FirstW(h, &entry)) {
    return 0;
  }
  do {
    if (entry.th32ProcessID == pid) {
      return entry.th32ParentProcessID;
    }
  } while(Process32NextW(h, &entry));
  return 0;
}

void WebExtHandler::proc_info(const QByteArray& msg) {
  Q_UNUSED(msg);

  DWORD ppid = getParentProcessId(GetCurrentProcessId());
  HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ppid);
  if (!h) {
    qWarning() << "Failed to locate parent process";
    return;
  }
  auto guard = qScopeGuard([h]() { CloseHandle(h); });

  QJsonObject obj;
  obj["pid"] = QJsonValue(static_cast<qint64>(ppid));

  wchar_t filename[MAX_PATH];
  DWORD sz = MAX_PATH;
  if (QueryFullProcessImageNameW(h, 0, filename, &sz)) {
    obj["exe"] = QJsonValue(QString::fromWCharArray(filename, sz));
  }

  writeJsonStdout(obj);
}

void WebExtHandler::start(const QByteArray& msg) {
  Q_UNUSED(msg);

  QProcess proc;
  QFileInfo exe("C:/Program Files/Mozilla/Mozilla VPN/Mozilla VPN.exe");
  proc.setProgram(exe.filePath());
  proc.setArguments(QStringList() << exe.fileName() << "ui");
  proc.setWorkingDirectory(exe.dir().path());

  QJsonObject obj;
  if (!proc.startDetached()) {
    qWarning() << "Failed to launch vpn client:" << proc.error();
    obj["error"] = "start_failed";
  } else {
    obj["status"] = "requested_start";
  }
  writeJsonStdout(obj);
}
