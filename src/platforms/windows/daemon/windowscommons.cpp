/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscommons.h"
#include "logger.h"

#include <QDir>
#include <QStandardPaths>

#include <Windows.h>

#define TUNNEL_SERVICE_NAME L"WireGuardTunnel$mozvpn"

namespace {
Logger logger(LOG_MAIN, "WindowsCommons");
}

// A simple function to log windows error messages.
void WindowsCommons::windowsLog(const QString& msg) {
  DWORD errorId = GetLastError();

  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, errorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, nullptr);

  std::string message(messageBuffer, size);

  logger.log() << msg << "-" << QString(message.c_str());
  LocalFree(messageBuffer);
}

QString WindowsCommons::tunnelConfigFile() {
  QStringList paths =
      QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
  for (const QString& path : paths) {
    QDir dir(path);
    if (!dir.exists()) {
      continue;
    }

    QDir mozillaDir(dir.filePath("Mozilla"));
    if (!mozillaDir.exists()) {
      continue;
    }

    QDir fpnDir(mozillaDir.filePath("FirefoxPrivateNetworkVPN"));
    if (!fpnDir.exists()) {
      continue;
    }

    QString wireguardFile(fpnDir.filePath("FirefoxPrivateNetworkVPN.conf"));
    if (!QFileInfo::exists(wireguardFile)) {
      continue;
    }

    logger.log() << "Found the current wireguard configuration:"
                 << wireguardFile;
    return wireguardFile;
  }

  for (const QString& path : paths) {
    QDir dir(path);

    QDir mozillaDir(dir.filePath("Mozilla"));
    if (!mozillaDir.exists() && !dir.mkdir("Mozilla")) {
      logger.log() << "Failed to create path Mozilla under" << path;
      continue;
    }

    QDir fpnDir(mozillaDir.filePath("FirefoxPrivateNetworkVPN"));
    if (!fpnDir.exists() && !mozillaDir.mkdir("FirefoxPrivateNetworkVPN")) {
      logger.log() << "Failed to create path FirefoxPrivateNetworkVPN under"
                   << mozillaDir.path();
      continue;
    }

    return fpnDir.filePath("FirefoxPrivateNetworkVPN.conf");
  }

  logger.log() << "Failed to create the right paths";
  return QString();
}
