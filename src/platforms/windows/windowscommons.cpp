/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscommons.h"
#include "logger.h"

#include <QDir>
#include <QtEndian>
#include <QHostAddress>
#include <QStandardPaths>
#include <QNetworkInterface>

#include <Windows.h>
#include <iphlpapi.h>

#define TUNNEL_SERVICE_NAME L"WireGuardTunnel$mozvpn"

constexpr const char* VPN_NAME = "MozillaVPN";

namespace {
Logger logger(LOG_MAIN, "WindowsCommons");
}

QString WindowsCommons::getErrorMessage() {
  DWORD errorId = GetLastError();
  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, errorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, nullptr);

  std::string message(messageBuffer, size);
  QString result(message.c_str());
  LocalFree(messageBuffer);
  return result;
}

// A simple function to log windows error messages.
void WindowsCommons::windowsLog(const QString& msg) {
  QString errmsg = getErrorMessage();
  logger.log() << msg << "-" << errmsg;
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

QString WindowsCommons::tunnelLogFile() {
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

    return vpnDir.filePath("log.bin");
  }

  return QString();
}

// static
int WindowsCommons::AdapterIndexTo(const QHostAddress& dst) {
  logger.log() << "Getting Current Internet Adapter that routes to"
               << dst.toString();
  quint32_be ipBigEndian;
  quint32 ip = dst.toIPv4Address();
  qToBigEndian(ip, &ipBigEndian);
  _MIB_IPFORWARDROW routeInfo;
  auto result = GetBestRoute(ipBigEndian, 0, &routeInfo);
  if (result != NO_ERROR) {
    return -1;
  }
  auto adapter =
      QNetworkInterface::interfaceFromIndex(routeInfo.dwForwardIfIndex);
  logger.log() << "Internet Adapter:" << adapter.name();
  return routeInfo.dwForwardIfIndex;
}

// static
int WindowsCommons::VPNAdapterIndex() {
  // For someReason QNetworkInterface::fromName(MozillaVPN) does not work >:(
  auto adapterList = QNetworkInterface::allInterfaces();
  for (const auto& adapter : adapterList) {
    if (adapter.humanReadableName().contains("MozillaVPN")) {
      return adapter.index();
    }
  }
  return -1;
}

// Static
QString WindowsCommons::getCurrentPath() {
  QByteArray buffer(2048, 0xFF);
  auto ok = GetModuleFileNameA(NULL, buffer.data(), buffer.size());

  if (ok == ERROR_INSUFFICIENT_BUFFER) {
    buffer.resize(buffer.size() * 2);
    ok = GetModuleFileNameA(NULL, buffer.data(), buffer.size());
  }
  if (ok == 0) {
    WindowsCommons::windowsLog("Err fetching dos path");
    return "";
  }
  QString::fromWCharArray((wchar_t*)buffer.data());
  return QString::fromLocal8Bit(buffer);
}