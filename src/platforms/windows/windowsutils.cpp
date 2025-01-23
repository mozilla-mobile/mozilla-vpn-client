/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsutils.h"

#include <Windows.h>
#include <dwmapi.h>
#include <errhandlingapi.h>
#include <qwindow.h>
#include <winsvc.h>

#include <QImage>
#include <QSettings>
#include <QSysInfo>
#include <QWindow>

#pragma comment(lib, "dwmapi.lib")

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

void WindowsUtils::setTitleBarIcon(QWindow* window, const QImage& icon) {
  auto const windowHandle = (HWND)window->winId();
  HICON hIcon = icon.toHICON();
  SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}
void WindowsUtils::setDockIcon(QWindow* window, const QImage& icon) {
  auto const windowHandle = (HWND)window->winId();
  HICON hIcon = icon.toHICON();
  SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

// constexpr to Help do color things in compile time.
//
namespace ColorUtils {
constexpr uint8_t hexToByte(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  throw std::invalid_argument("Invalid hex character");
}

constexpr uint8_t parseHexByte(const char* hex) {
  return (hexToByte(hex[0]) << 4) | hexToByte(hex[1]);
}
/**
 * @brief converts a hex RGBA to AGBR represented as DWORD
 * Binary format is 0x00BBGGRR
 */
constexpr uint32_t toCOLORREF(const char* color) {
  // Ensure it's a valid format
  if (color[0] != '#' || color[7] != '\0') {
    throw std::invalid_argument("Invalid color format. Expected '#RRGGBB'.");
  }

  // Extract and convert R, G, B
  uint8_t r = parseHexByte(&color[1]);
  uint8_t g = parseHexByte(&color[3]);
  uint8_t b = parseHexByte(&color[5]);

  // Construct and return the COLORREF value in 0x00BBGGRR format
  return (b << 16) | (g << 8) | r;
}
}  // namespace ColorUtils

void WindowsUtils::updateTitleBarColor(QWindow* window, bool darkMode) {
  // TODO: Fetch that from the theme data.
  const COLORREF defaultColor = darkMode ? ColorUtils::toCOLORREF("#0C0C0D")
                                         : ColorUtils::toCOLORREF("#F9F9FA");

  auto const windowHandle = (HWND)window->winId();
  auto const ok = SUCCEEDED(DwmSetWindowAttribute(
      windowHandle, DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, &defaultColor,
      sizeof(defaultColor)));
  Q_ASSERT(ok);
}

void WindowsUtils::forceWindowRedraw(QWindow* w) {
  auto const windowHandle = (HWND)w->winId();
  ShowWindow(windowHandle, SW_MINIMIZE);
  ShowWindow(windowHandle, SW_RESTORE);
}
