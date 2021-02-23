/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscommons.h"
#include "logger.h"

#include <QDir>
#include <QStandardPaths>

#include <Windows.h>

#define TUNNEL_SERVICE_NAME L"WireGuardTunnel$mozvpn"

constexpr const char *VPN_NAME = "MozillaVPN";

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

    QDir vpnDir(dir.filePath(VPN_NAME));
    if (!vpnDir.exists()) {
      continue;
    }

    QString wireguardFile(vpnDir.filePath(QString("%1.conf").arg(VPN_NAME)));
    if (!QFileInfo::exists(wireguardFile)) {
      continue;
    }

    logger.log() << "Found the current wireguard configuration:"
                 << wireguardFile;
    return wireguardFile;
  }

  for (const QString& path : paths) {
    QDir dir(path);

    QDir vpnDir(dir.filePath(VPN_NAME));
    if (!vpnDir.exists() && !dir.mkdir(VPN_NAME)) {
      logger.log() << "Failed to create path Mozilla under" << path;
      continue;
    }

    return vpnDir.filePath(QString("%1.conf").arg(VPN_NAME));
  }

  logger.log() << "Failed to create the right paths";
  return QString();
}
