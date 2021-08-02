/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemontunnel.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"

#include <QCoreApplication>

#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WindowsDaemonTunnel");
}  // namespace

WindowsDaemonTunnel::WindowsDaemonTunnel(QObject* parent)
    : Command(parent, "tunneldaemon", "Activate the windows tunnel service") {
  MVPN_COUNT_CTOR(WindowsDaemonTunnel);
}

WindowsDaemonTunnel::~WindowsDaemonTunnel() {
  MVPN_COUNT_DTOR(WindowsDaemonTunnel);
}

int WindowsDaemonTunnel::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());

  logger.log() << "Tunnel daemon service is starting";

  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN Tunnel");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  if (tokens.length() != 2) {
    logger.error() << "Expected 1 parameter only: the config file.";
    return 1;
  }

  // This process will be used by the wireguard tunnel. No need to call
  // FreeLibrary.
  HMODULE tunnelLib = LoadLibrary(TEXT("tunnel.dll"));
  if (!tunnelLib) {
    WindowsCommons::windowsLog("Failed to load tunnel.dll");
    return 1;
  }

  typedef bool WireGuardTunnelService(const ushort* settings);

  WireGuardTunnelService* tunnelProc = (WireGuardTunnelService*)GetProcAddress(
      tunnelLib, "WireGuardTunnelService");
  if (!tunnelProc) {
    WindowsCommons::windowsLog("Failed to get WireGuardTunnelService function");
    return 1;
  }

  QString configFile = WindowsCommons::tunnelConfigFile();
  if (configFile.isEmpty()) {
    logger.error() << "Failed to retrieve the config file";
    return 1;
  }

  if (!tunnelProc(configFile.utf16())) {
    logger.error() << "Failed to activate the tunnel service";
    return 1;
  }

  return 0;
}

static Command::RegistrationProxy<WindowsDaemonTunnel>
    s_commandWindowsDaemonTunnel;
