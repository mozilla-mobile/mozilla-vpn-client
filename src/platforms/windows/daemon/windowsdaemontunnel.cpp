/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemontunnel.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "logger.h"
#include "windowscommons.h"

#include <QCoreApplication>

#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WindowsDaemonTunnel");

void tunnelLoggerFunc(int level, const char* msg) {
  Q_UNUSED(level);
  logger.log() << "tunnel.dll:" << msg;
}

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
    logger.log() << "Expected 1 parameter only: the config file.";
    return 1;
  }

  // This process will be used by the wireguard tunnel. No need to call
  // FreeLibrary.
  HMODULE tunnelLib = LoadLibrary(TEXT("tunnel.dll"));
  if (!tunnelLib) {
    WindowsCommons::windowsLog("Failed to load tunnel.dll");
    return 1;
  }

  typedef struct {
    const char* p;
    size_t n;
  } gostring_t;

  typedef void (*logFunc)(int level, const char* msg);
  typedef bool WireGuardTunnelService(gostring_t settings);
  typedef void WireGuardTunnelLogger(logFunc func);

  WireGuardTunnelLogger* tunnelLogger = (WireGuardTunnelLogger*)GetProcAddress(
      tunnelLib, "WireGuardTunnelLogger");
  if (!tunnelLogger) {
    WindowsCommons::windowsLog("Failed to get WireGuardTunnelLogger function");
    return 1;
  }

  WireGuardTunnelService* tunnelProc = (WireGuardTunnelService*)GetProcAddress(
      tunnelLib, "WireGuardTunnelService");
  if (!tunnelProc) {
    WindowsCommons::windowsLog("Failed to get WireGuardTunnelService function");
    return 1;
  }

  tunnelLogger(tunnelLoggerFunc);

  QString configFile = WindowsCommons::tunnelConfigFile();
  if (configFile.isEmpty()) {
    logger.log() << "Failed to retrieve the config file";
    return 1;
  }

  QByteArray configFileData = configFile.toLocal8Bit();
  gostring_t goConfigFile;
  goConfigFile.p = configFileData.constData();
  goConfigFile.n = configFileData.length();

  if (!tunnelProc(goConfigFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return 1;
  }

  return 0;
}

static Command::RegistrationProxy<WindowsDaemonTunnel>
    s_commandWindowsDaemonTunnel;
