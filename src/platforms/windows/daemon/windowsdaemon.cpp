/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "wgquickprocess.h"
#include "windowscommons.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QScopeGuard>
#include <QTextStream>
#include <QtGlobal>

#include <Windows.h>

#define TUNNEL_SERVICE_NAME L"WireGuardTunnel$MozillaVPN"
#define TUNNEL_NAMED_PIPE                                                      \
  "\\\\."                                                                      \
  "\\pipe\\ProtectedPrefix\\Administrators\\WireGuard\\FirefoxPrivateNetworkV" \
  "PN"

namespace {
Logger logger(LOG_WINDOWS, "WindowsDaemon");
WindowsDaemon* s_daemon = nullptr;

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

}  // namespace

WindowsDaemon::WindowsDaemon() : Daemon(nullptr) {
  MVPN_COUNT_CTOR(WindowsDaemon);

  logger.log() << "Daemon created";

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;
}

WindowsDaemon::~WindowsDaemon() {
  MVPN_COUNT_DTOR(WindowsDaemon);

  logger.log() << "Daemon released";

  m_state = Inactive;

  stopAndDeleteTunnelService();

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

// static
WindowsDaemon* WindowsDaemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
}

bool WindowsDaemon::activate(const Config& config) {
  if (!Daemon::activate(config)) {
    return false;
  }

  m_connectionDate = QDateTime::currentDateTime();
  return true;
}

void WindowsDaemon::status(QLocalSocket* socket) {
  logger.log() << "Status request";
  Q_ASSERT(socket);

  QJsonObject obj;
  obj.insert("type", "status");
  obj.insert("connected", m_connected);

  HANDLE pipe = INVALID_HANDLE_VALUE;

  auto guard = qScopeGuard([&] {
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    socket->write("\n");

    if (pipe != INVALID_HANDLE_VALUE) {
      CloseHandle(pipe);
    }
  });

  if (!m_connected) {
    return;
  }

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
      return;
    }

    logger.log() << "Pipes are busy. Let's wait";

    if (!WaitNamedPipe(tunnelName, 1000)) {
      WindowsCommons::windowsLog("Failed to wait for named pipes");
      return;
    }

    ++tries;
  }

  DWORD mode = PIPE_READMODE_MESSAGE;
  if (SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr)) {
    WindowsCommons::windowsLog("Failed to set the read-mode on pipe");
    return;
  }

  QByteArray message = "get=1\n\n";
  DWORD written;

  if (!WriteFile(pipe, message.constData(), message.length(), &written,
                 nullptr)) {
    WindowsCommons::windowsLog("Failed to write into the pipe");
    return;
  }

  QByteArray data;
  while (true) {
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

    if (steps >= (TxFound & RxFound)) {
      break;
    }
  }

  guard.dismiss();

  obj.insert("status", true);
  obj.insert("serverIpv4Gateway", m_lastConfig.m_serverIpv4Gateway);
  obj.insert("date", m_connectionDate.toString());
  obj.insert("txBytes", QJsonValue(double(txBytes)));
  obj.insert("rxBytes", QJsonValue(double(rxBytes)));

  socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  socket->write("\n");
}

void WindowsDaemon::logs(QLocalSocket* socket) {
  logger.log() << "Log request";

  Q_ASSERT(socket);

  QJsonObject obj;
  obj.insert("type", "logs");
  obj.insert("logs", Daemon::logs().replace("\n", "|"));
  socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  socket->write("\n");
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
    return true;
  }

  logger.log() << "Failed to run the tunnel service";

  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    WindowsCommons::windowsLog("Failed to retrieve the service status");
    return false;
  }

  // TODO expose the status.dwWin32ExitCode
  return false;
}

bool WindowsDaemon::run(Daemon::Op op, const Config& config) {
  logger.log() << (op == Daemon::Up ? "Activate" : "Deactivate") << "the vpn";

  if (op == Daemon::Down && m_state == Inactive) {
    logger.log() << "Nothing to do. The tunnel service is down";
    return true;
  }

  if (op == Daemon::Down) {
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

  if (!WgQuickProcess::createConfigFile(
          tunnelFile, config.m_privateKey, config.m_deviceIpv4Address,
          config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
          config.m_serverIpv6Gateway, config.m_serverPublicKey,
          config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
          config.m_allowedIPAddressRanges.join(", "), config.m_serverPort,
          config.m_ipv6Enabled)) {
    logger.log() << "Failed to create a config file";
    return false;
  }

  if (!registerTunnelService(tunnelFile)) {
    logger.log() << "Failed to activate the tunnel service";
    return false;
  }

  logger.log() << "Registration completed";

  m_state = Active;
  return true;
}
