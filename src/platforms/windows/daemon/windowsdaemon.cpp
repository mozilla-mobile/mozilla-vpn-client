/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "wgquickprocess.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QScopeGuard>
#include <QTextStream>
#include <QtGlobal>

#include <Windows.h>

#define TUNNEL_NAMED_PIPE \
  "\\\\."                 \
  "\\pipe\\ProtectedPrefix\\Administrators\\WireGuard\\MozillaVPN"

namespace {

Logger logger(LOG_WINDOWS, "WindowsDaemon");

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

bool stopAndDeleteTunnelService() {
  SC_HANDLE scm = nullptr;
  SC_HANDLE service = nullptr;

  auto closeService = qScopeGuard([&] {
    if (service) {
      CloseServiceHandle(service);
    }
    if (scm) {
      CloseServiceHandle(scm);
    }
  });

  scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (!scm) {
    WindowsCommons::windowsLog("Failed to open the SCM");
    return false;
  }

  service = OpenService(scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
  if (!service) {
    WindowsCommons::windowsLog("Failed to open the service");
    return false;
  }

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

HANDLE createPipe() {
  HANDLE pipe = INVALID_HANDLE_VALUE;

  auto guard = qScopeGuard([&] {
    if (pipe != INVALID_HANDLE_VALUE) {
      CloseHandle(pipe);
    }
  });

  LPTSTR tunnelName = (LPTSTR)TEXT(TUNNEL_NAMED_PIPE);

  uint32_t tries = 0;
  while (tries < 30) {
    pipe = CreateFile(tunnelName, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                      OPEN_EXISTING, 0, nullptr);

    if (pipe != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      WindowsCommons::windowsLog("Failed to create a named pipe");
      return INVALID_HANDLE_VALUE;
    }

    logger.log() << "Pipes are busy. Let's wait";

    if (!WaitNamedPipe(tunnelName, 1000)) {
      WindowsCommons::windowsLog("Failed to wait for named pipes");
      return INVALID_HANDLE_VALUE;
    }

    ++tries;
  }

  DWORD mode = PIPE_READMODE_BYTE;
  if (!SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr)) {
    WindowsCommons::windowsLog("Failed to set the read-mode on pipe");
    return INVALID_HANDLE_VALUE;
  }

  guard.dismiss();
  return pipe;
}

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

}  // namespace

WindowsDaemon::WindowsDaemon() : Daemon(nullptr) {
  MVPN_COUNT_CTOR(WindowsDaemon);

  connect(&m_tunnelMonitor, &WindowsTunnelMonitor::backendFailure, this,
          &WindowsDaemon::monitorBackendFailure);
}

WindowsDaemon::~WindowsDaemon() {
  MVPN_COUNT_DTOR(WindowsDaemon);

  logger.log() << "Daemon released";

  m_state = Inactive;

  stopAndDeleteTunnelService();
}

QByteArray WindowsDaemon::getStatus() {
  logger.log() << "Status request";

  QJsonObject obj;
  obj.insert("type", "status");
  obj.insert("connected", m_connected);

  HANDLE pipe = createPipe();

  auto guard = qScopeGuard([&] {
    if (pipe != INVALID_HANDLE_VALUE) {
      CloseHandle(pipe);
    }
  });

  if (pipe == INVALID_HANDLE_VALUE || !m_connected) {
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
  }

  QByteArray message = "get=1\n\n";
  DWORD written;

  if (!WriteFile(pipe, message.constData(), message.length(), &written,
                 nullptr)) {
    WindowsCommons::windowsLog("Failed to write into the pipe");
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
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

  uint64_t txBytes = 0;
  uint64_t rxBytes = 0;

  int steps = 0;
  constexpr const int TxFound = 0x01;
  constexpr const int RxFound = 0x02;

  for (const QByteArray& line : data.split('\n')) {
    if (!line.contains('=')) {
      continue;
    }

    QList<QByteArray> parts = line.split('=');
    if (parts[0] == "tx_bytes") {
      txBytes = parts[1].toLongLong();
      steps |= TxFound;
    } else if (parts[0] == "rx_bytes") {
      rxBytes = parts[1].toLongLong();
      steps |= RxFound;
    }

    if (steps >= (TxFound | RxFound)) {
      break;
    }
  }

  obj.insert("status", true);
  obj.insert("serverIpv4Gateway", m_lastConfig.m_serverIpv4Gateway);
  obj.insert("deviceIpv4Address", m_lastConfig.m_deviceIpv4Address);
  obj.insert("date", m_connectionDate.toString());
  obj.insert("txBytes", QJsonValue(double(txBytes)));
  obj.insert("rxBytes", QJsonValue(double(rxBytes)));

  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

bool WindowsDaemon::registerTunnelService(const QString& configFile) {
  logger.log() << "Register tunnel service";

  SC_HANDLE scm;
  SC_HANDLE service;

  auto guard = qScopeGuard([&] {
    if (service) {
      CloseServiceHandle(service);
    }

    if (scm) {
      CloseServiceHandle(scm);
    }
  });

  scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (!scm) {
    WindowsCommons::windowsLog("Failed to open SCManager");
    return false;
  }

  // Let's see if we have to delete a previous instance.
  service = OpenService(scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
  if (service) {
    logger.log() << "An existing service has been detected. Let's close it.";

    CloseServiceHandle(service);
    service = nullptr;

    if (!stopAndDeleteTunnelService()) {
      return false;
    }
  }

  QString servicePath;
  {
    QTextStream out(&servicePath);
    out << "\"" << qApp->applicationFilePath() << "\" tunneldaemon \""
        << configFile << "\"";
  }

  logger.log() << "Service name:" << servicePath;

  service = CreateService(scm, TUNNEL_SERVICE_NAME, L"Mozilla VPN (tunnel)",
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

  emit backendFailure();
  return false;
}

bool WindowsDaemon::run(Daemon::Op op, const InterfaceConfig& config) {
  logger.log() << (op == Daemon::Up ? "Activate" : "Deactivate") << "the vpn";

  if (op == Daemon::Down && m_state == Inactive) {
    logger.log() << "Nothing to do. The tunnel service is down";
    return true;
  }

  if (op == Daemon::Down) {
    m_tunnelMonitor.stop();
    stopAndDeleteTunnelService();
    m_state = Inactive;
    return true;
  }

  Q_ASSERT(op == Daemon::Up);

  if (m_state == Active) {
    logger.log() << "Nothing to do. The tunnel service is up";
    return true;
  }

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
  m_tunnelMonitor.resetLogs();

  if (!registerTunnelService(tunnelFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return false;
  }

  logger.log() << "Registration completed";

  m_tunnelMonitor.start();
  m_state = Active;
  return true;
}

bool WindowsDaemon::supportServerSwitching(
    const InterfaceConfig& config) const {
  return m_lastConfig.m_privateKey == config.m_privateKey &&
         m_lastConfig.m_deviceIpv4Address == config.m_deviceIpv4Address &&
         m_lastConfig.m_deviceIpv6Address == config.m_deviceIpv6Address &&
         m_lastConfig.m_serverIpv4Gateway == config.m_serverIpv4Gateway &&
         m_lastConfig.m_serverIpv6Gateway == config.m_serverIpv6Gateway;
}

bool WindowsDaemon::switchServer(const InterfaceConfig& config) {
  logger.log() << "Switching server";

  Q_ASSERT(m_connected);

  HANDLE pipe = createPipe();

  auto guard = qScopeGuard([&] {
    if (pipe != INVALID_HANDLE_VALUE) {
      CloseHandle(pipe);
    }
  });

  if (pipe == INVALID_HANDLE_VALUE) {
    return false;
  }

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

void WindowsDaemon::monitorBackendFailure() {
  logger.log() << "Tunnel service is down";

  m_tunnelMonitor.stop();

  emit backendFailure();
  deactivate();

  m_state = Inactive;
}
