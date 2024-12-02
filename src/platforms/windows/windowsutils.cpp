/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsutils.h"

#include <Windows.h>
#include <errhandlingapi.h>
#include <winsvc.h>

#include <QSettings>
#include <QSysInfo>

#include "logger.h"

namespace {
Logger logger("WindowsUtils");
}  // namespace

QString WindowsUtils::getErrorMessage(quint32 code) {
  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, nullptr);

  std::string message(messageBuffer, size);
  QString result(message.c_str());
  LocalFree(messageBuffer);
  return result;
}

QString WindowsUtils::getErrorMessage() {
  return getErrorMessage(GetLastError());
}

// A simple function to log windows error messages.
void WindowsUtils::windowsLog(const QString& msg) {
  QString errmsg = getErrorMessage();
  logger.error() << msg << "-" << errmsg;
}

// Static
QString WindowsUtils::windowsVersion() {
  QSettings regCurrentVersion(
      "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
      QSettings::NativeFormat);

  return regCurrentVersion.value("CurrentBuild").toString();
}

// static
void WindowsUtils::forceCrash() {
  RaiseException(0x0000DEAD, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

// static
bool WindowsUtils::getServiceStatus(const QString& name) {
  auto scmRights = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE |
                   SC_MANAGER_QUERY_LOCK_STATUS | SERVICE_QUERY_STATUS;
  auto scm = OpenSCManager(nullptr, nullptr, scmRights);
  auto guard = qScopeGuard([scm]() { CloseServiceHandle(scm); });
  if (scm == nullptr) {
    logger.error() << " OpenSCManager failed: " << getErrorMessage();
    return false;
  }

  auto service = OpenService(scm, (LPCWSTR)name.utf16(), SERVICE_QUERY_STATUS);
  if (service == nullptr) {
    logger.error() << " OpenService failed: " << getErrorMessage();
    return false;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    logger.error() << " QueryServiceStatus failed: " << getErrorMessage();
    return false;
  }
  return (status.dwCurrentState == SERVICE_RUNNING);
}
