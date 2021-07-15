/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilswindows.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "windowsdaemon.h"
#include "wgquickprocess.h"

#include <QScopeGuard>

#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WireguardUtilsWindows");
}; // namespace

WireguardUtilsWindows::WireguardUtilsWindows(QObject* parent)
    : WireguardUtils(parent), m_tunnel(this) {
  MVPN_COUNT_CTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows created.";
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MVPN_COUNT_DTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows destroyed.";
}

WireguardUtils::peerBytes WireguardUtilsWindows::getThroughputForInterface() {
  peerBytes pb = {0,0};
  HANDLE pipe = m_tunnel.createPipe();
  if (pipe == INVALID_HANDLE_VALUE) {
    return pb;
  }
  auto guard = qScopeGuard([&] {
    CloseHandle(pipe);
  });

  QByteArray message = "get=1\n\n";
  DWORD written;

  if (!WriteFile(pipe, message.constData(), message.length(), &written,
                 nullptr)) {
    WindowsCommons::windowsLog("Failed to write into the pipe");
    return pb;
  }

  QByteArray data;
  while (!data.contains("\n\n")) {
    char buffer[512];
    DWORD read = 0;
    if (!ReadFile(pipe, buffer, sizeof(buffer), &read, nullptr)) {
      break;
    }

    data.append(buffer, read);
  }

  int steps = 0;
  constexpr const int TxFound = 0x01;
  constexpr const int RxFound = 0x02;

  for (const QByteArray& line : data.split('\n')) {
    if (!line.contains('=')) {
      continue;
    }

    QList<QByteArray> parts = line.split('=');
    if (parts[0] == "tx_bytes") {
      pb.txBytes = parts[1].toDouble();
      steps |= TxFound;
    } else if (parts[0] == "rx_bytes") {
      pb.rxBytes = parts[1].toDouble();
      steps |= RxFound;
    }

    if (steps >= (TxFound | RxFound)) {
      return pb;
    }
  }
  
  return pb;
}

bool WireguardUtilsWindows::addInterface(const InterfaceConfig& config) {
  QString tunnelFile = WindowsCommons::tunnelConfigFile();
  if (tunnelFile.isEmpty()) {
    logger.log() << "Failed to choose the tunnel config file";
    return false;
  }

  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  if (!WgQuickProcess::createConfigFile(
          tunnelFile, config.m_privateKey, config.m_deviceIpv4Address,
          config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
          config.m_serverIpv6Gateway, config.m_serverPublicKey,
          config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
          addresses.join(", "), config.m_serverPort, config.m_ipv6Enabled,
          config.m_dnsServer)) {
    logger.log() << "Failed to create a config file";
    return false;
  }

  m_tunnel.resetLogs();
  if (!m_tunnel.start(tunnelFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return false;
  }

  logger.log() << "Registration completed";
  //m_state = Active;
  return true;
}

bool WireguardUtilsWindows::deleteInterface() {
  m_tunnel.stop();
  return true;
}

// Dummy implementations for now
bool WireguardUtilsWindows::updateInterface(const InterfaceConfig& config) {
  HANDLE pipe = m_tunnel.createPipe();
  if (pipe == INVALID_HANDLE_VALUE) {
    return false;
  }
  auto guard = qScopeGuard([&] {
    CloseHandle(pipe);
  });

  QByteArray message;
  {
    QTextStream out(&message);
    out << "set=1\n";
    out << "replace_peers=true\n";

    QByteArray publicKey =
        QByteArray::fromBase64(config.m_serverPublicKey.toLocal8Bit()).toHex();
    out << "public_key=" << publicKey << "\n";

    out << "endpoint=" << config.m_serverIpv4AddrIn << ":"
        << config.m_serverPort << "\n";

    for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
      out << "allowed_ip=" << ip.toString() << "\n";
    }

    out << "\n";
  }

  DWORD written;
  if (!WriteFile(pipe, message.constData(), message.length(), &written,
                 nullptr)) {
    WindowsCommons::windowsLog("Failed to write into the pipe");
    return false;
  }

  QByteArray data;
  while (!data.contains("\n\n")) {
    char buffer[512];
    DWORD read = 0;
    if (!ReadFile(pipe, buffer, sizeof(buffer), &read, nullptr)) {
      break;
    }

    data.append(buffer, read);
  }

  logger.log() << "DATA:" << data;
  guard.dismiss();
  return true;
}

bool WireguardUtilsWindows::addRoutePrefix(const IPAddressRange& prefix) {
  Q_UNUSED(prefix);
  return true;
}
