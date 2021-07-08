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

QString exitCodeToFailure(DWORD exitCode) {
  // The order of this error code is taken from wireguard.
  switch (exitCode) {
    case 0:
      return "No error";
    case 1:
      return "Error when opening the ringlogger log file";
    case 2:
      return "Error while loading the WireGuard configuration file from "
             "path.";
    case 3:
      return "Error while creating a WinTun device.";
    case 4:
      return "Error while listening on a named pipe.";
    case 5:
      return "Error while resolving DNS hostname endpoints.";
    case 6:
      return "Error while manipulating firewall rules.";
    case 7:
      return "Error while setting the device configuration.";
    case 8:
      return "Error while binding sockets to default routes.";
    case 9:
      return "Unable to set interface addresses, routes, dns, and/or "
             "interface settings.";
    case 10:
      return "Error while determining current executable path.";
    case 11:
      return "Error while opening the NUL file.";
    case 12:
      return "Error while attempting to track tunnels.";
    case 13:
      return "Error while attempting to enumerate current sessions.";
    case 14:
      return "Error while dropping privileges.";
    case 15:
      return "Windows internal error.";
    default:
      return "Unknown error";
  }
}
}; // namespace

WireguardUtilsWindows::WireguardUtilsWindows(QObject* parent)
    : WireguardUtils(parent) {
  MVPN_COUNT_CTOR(WireguardUtilsWindows);

  m_scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (!m_scm) {
    WindowsCommons::windowsLog("Failed to open SCManager");
  }

  logger.log() << "WireguardUtilsWindows created.";
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MVPN_COUNT_DTOR(WireguardUtilsWindows);

  if (m_service) {
    stopAndDeleteTunnelService(m_service);
    CloseServiceHandle(m_service);
  }

  CloseServiceHandle(m_scm);
  logger.log() << "WireguardUtilsWindows destroyed.";
}

bool WireguardUtilsWindows::interfaceExists() {
  if (m_service == nullptr) {
    return false;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus(m_service, &status)) {
    return false;
  }

  return (status.dwCurrentState == SERVICE_RUNNING);
}

WireguardUtils::peerBytes WireguardUtilsWindows::getThroughputForInterface() {
  peerBytes pb = {0,0};
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
  //m_tunnelMonitor.resetLogs();

  if (!registerTunnelService(tunnelFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return false;
  }

  logger.log() << "Registration completed";

  //m_tunnelMonitor.start();
  //m_state = Active;
  return true;
}

bool WireguardUtilsWindows::deleteInterface() {
  if (m_service == nullptr) {
    return false;
  }
  stopAndDeleteTunnelService(m_service);
  CloseServiceHandle(m_service);
  m_service = nullptr;
  return true;
}

// Dummy implementations for now
bool WireguardUtilsWindows::updateInterface(const InterfaceConfig& config) {
  Q_UNUSED(config);
  return true;
}


bool WireguardUtilsWindows::addRoutePrefix(const IPAddressRange& prefix) {
  Q_UNUSED(prefix);
  return true;
}

bool waitForServiceStatus(SC_HANDLE service, DWORD expectedStatus) {
  int tries = 0;
  while (tries < 30) {
    SERVICE_STATUS status;
    if (!QueryServiceStatus(service, &status)) {
      WindowsCommons::windowsLog("Failed to retrieve the service status");
      return false;
    }

    if (status.dwCurrentState == expectedStatus) {
      return true;
    }

    logger.log() << "The service is not in the right status yet.";

    Sleep(1000);
    ++tries;
  }

  return false;
}

bool WireguardUtilsWindows::registerTunnelService(const QString& configFile) {
  logger.log() << "Register tunnel service";

  SC_HANDLE service;

  auto guard = qScopeGuard([&] {
    if (service) {
      CloseServiceHandle(service);
    }
  });

  // Let's see if we have to delete a previous instance.
  service = OpenService(m_scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
  if (service) {
    logger.log() << "An existing service has been detected. Let's close it.";
    if (!stopAndDeleteTunnelService(service)) {
      return false;
    }
    CloseServiceHandle(service);
    service = nullptr;
  }

  QString servicePath;
  {
    QTextStream out(&servicePath);
    out << "\"" << qApp->applicationFilePath() << "\" tunneldaemon \""
        << configFile << "\"";
  }

  logger.log() << "Service name:" << servicePath;

  service = CreateService(m_scm, TUNNEL_SERVICE_NAME, L"Mozilla VPN (tunnel)",
                          SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                          SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                          (const wchar_t*)servicePath.utf16(), nullptr, 0,
                          TEXT("Nsi\0TcpIp\0"), nullptr, nullptr);
  if (!service) {
    WindowsCommons::windowsLog("Failed to create the tunnel service");
    return false;
  }

  SERVICE_DESCRIPTION sd = {
      (wchar_t*)L"Manages the Mozilla VPN tunnel connection"};

  if (!ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &sd)) {
    WindowsCommons::windowsLog(
        "Failed to set the description to the tunnel service");
    return false;
  }

  SERVICE_SID_INFO ssi;
  ssi.dwServiceSidType = SERVICE_SID_TYPE_UNRESTRICTED;
  if (!ChangeServiceConfig2(service, SERVICE_CONFIG_SERVICE_SID_INFO, &ssi)) {
    WindowsCommons::windowsLog("Failed to set the SID to the tunnel service");
    return false;
  }

  if (!StartService(service, 0, nullptr)) {
    WindowsCommons::windowsLog("Failed to start the service");
    return false;
  }

  if (waitForServiceStatus(service, SERVICE_RUNNING)) {
    logger.log() << "The tunnel service is up and running";
    guard.dismiss();
    m_service = service;
    return true;
  }

  logger.log() << "Failed to run the tunnel service";

  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    WindowsCommons::windowsLog("Failed to retrieve the service status");
    return false;
  }

  logger.log() << "The tunnel service exits with status code:"
               << status.dwWin32ExitCode << "-"
               << exitCodeToFailure(status.dwWin32ExitCode);

  //emit backendFailure();
  return false;
}

bool WireguardUtilsWindows::stopAndDeleteTunnelService(SC_HANDLE service) {
  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    WindowsCommons::windowsLog("Failed to retrieve the service status");
    return false;
  }

  logger.log() << "The current service is stopped:"
               << (status.dwCurrentState == SERVICE_STOPPED);

  if (status.dwCurrentState != SERVICE_STOPPED) {
    logger.log() << "The service is not stopped yet.";
    if (!ControlService(service, SERVICE_CONTROL_STOP, &status)) {
      WindowsCommons::windowsLog("Failed to control the service");
      return false;
    }

    if (!waitForServiceStatus(service, SERVICE_STOPPED)) {
      logger.log() << "Unable to stop the service";
      return false;
    }
  }

  logger.log() << "Proceeding with the deletion";

  if (!DeleteService(service)) {
    WindowsCommons::windowsLog("Failed to delete the service");
    return false;
  }

  return true;
}
