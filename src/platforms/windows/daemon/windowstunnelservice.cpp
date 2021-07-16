/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WindowsTunnelService.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "windowsdaemon.h"

#include <QDateTime>
#include <QScopeGuard>

#include <Windows.h>

#define TUNNEL_NAMED_PIPE \
  "\\\\."                 \
  "\\pipe\\ProtectedPrefix\\Administrators\\WireGuard\\MozillaVPN"

constexpr uint32_t WINDOWS_TUNNEL_MONITOR_TIMEOUT_MSEC = 2000;

/* The ring logger format used by the Wireguard DLL is as follows, assuming
 * no padding:
 *
 * struct {
 *   uint32_t magic;
 *   uint32_t index;
 *   struct {
 *     uint64_t timestamp;
 *     char message[512];
 *   } ring[2048];
 * };
 */

constexpr uint32_t RINGLOG_POLL_MSEC = 250;
constexpr uint32_t RINGLOG_MAGIC_HEADER = 0xbadbabe;
constexpr uint32_t RINGLOG_INDEX_OFFSET = 4;
constexpr uint32_t RINGLOG_HEADER_SIZE = 8;
constexpr uint32_t RINGLOG_MAX_ENTRIES = 2048;
constexpr uint32_t RINGLOG_MESSAGE_SIZE = 512;
constexpr uint32_t RINGLOG_TIMESTAMP_SIZE = 8;
constexpr uint32_t RINGLOG_FILE_SIZE =
    RINGLOG_HEADER_SIZE +
    ((RINGLOG_MESSAGE_SIZE + RINGLOG_TIMESTAMP_SIZE) * RINGLOG_MAX_ENTRIES);

namespace {
Logger logger(LOG_WINDOWS, "WindowsTunnelService");
Logger logdll(LOG_WINDOWS, "tunnel.dll");
}  // namespace

static bool stopAndDeleteTunnelService(SC_HANDLE service);
static bool waitForServiceStatus(SC_HANDLE service, DWORD expectedStatus);

WindowsTunnelService::WindowsTunnelService(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(WindowsTunnelService);

  m_scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (m_scm == nullptr) {
    WindowsCommons::windowsLog("Failed to open SCManager");
  }

  m_logEpochNsec = QDateTime::currentMSecsSinceEpoch() * 1000000;

  connect(&m_timer, &QTimer::timeout, this, &WindowsTunnelService::timeout);
  connect(&m_logtimer, &QTimer::timeout, this,
          &WindowsTunnelService::processLogs);
}

WindowsTunnelService::~WindowsTunnelService() {
  MVPN_COUNT_CTOR(WindowsTunnelService);
  stop();
  CloseServiceHandle((SC_HANDLE)m_scm);
}

bool WindowsTunnelService::start(const QString& configFile) {
  logger.log() << "Starting the tunnel service";
  m_logEpochNsec = QDateTime::currentMSecsSinceEpoch() * 1000000;

  if (!registerTunnelService(configFile)) {
    return false;
  }
  m_timer.start(WINDOWS_TUNNEL_MONITOR_TIMEOUT_MSEC);

  /* Open and map the tunnel log file. */
  QString logFileName = WindowsCommons::tunnelLogFile();
  if (logFileName.isNull()) {
    return true;
  }
  m_logfile = new QFile(logFileName, this);
  m_logfile->open(QIODevice::ReadOnly);
  m_logindex = -1;
  m_logdata = m_logfile->map(0, RINGLOG_FILE_SIZE);
  if (!m_logdata) {
    return true;
  }

  /* Check for a valid magic header */
  uint32_t magic;
  memcpy(&magic, m_logdata, 4);
  logger.log() << "Opening tunnel log file" << logFileName;
  if (magic != RINGLOG_MAGIC_HEADER) {
    logger.log() << "Unexpected magic header:" << QString::number(magic, 16);
    return true;
  }

  m_logtimer.start(RINGLOG_POLL_MSEC);
  return true;
}

void WindowsTunnelService::stop() {
  SC_HANDLE service = (SC_HANDLE)m_service;
  if (service) {
    stopAndDeleteTunnelService(service);
    CloseServiceHandle(service);
    m_service = nullptr;
  }

  m_timer.stop();
  m_logtimer.stop();

  if (m_logfile) {
    m_logfile->unmap(m_logdata);
    delete m_logfile;
    m_logfile = nullptr;
    m_logdata = nullptr;
  }
}

bool WindowsTunnelService::isRunning() {
  if (m_service == nullptr) {
    return false;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus((SC_HANDLE)m_service, &status)) {
    return false;
  }

  return status.dwCurrentState == SERVICE_RUNNING;
}

void WindowsTunnelService::timeout() {
  if (m_service == nullptr) {
    logger.log() << "The service doesn't exist";
    emit backendFailure();
    return;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus((SC_HANDLE)m_service, &status)) {
    WindowsCommons::windowsLog("Failed to retrieve the service status");
    emit backendFailure();
    return;
  }

  if (status.dwCurrentState == SERVICE_RUNNING) {
    // The service is active
    return;
  }

  logger.log() << "The service is not active";
  emit backendFailure();
}

int WindowsTunnelService::nextLogIndex() {
  if (!m_logdata) {
    return 0;
  }
  qint32 value;
  memcpy(&value, m_logdata + RINGLOG_INDEX_OFFSET, 4);
  return value % RINGLOG_MAX_ENTRIES;
}

void WindowsTunnelService::processMessage(int index) {
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < RINGLOG_MAX_ENTRIES);
  size_t offset = index * (RINGLOG_TIMESTAMP_SIZE + RINGLOG_MESSAGE_SIZE);
  uchar* data = m_logdata + RINGLOG_HEADER_SIZE + offset;

  quint64 timestamp;
  memcpy(&timestamp, data, 8);
  if (timestamp <= m_logEpochNsec) {
    return;
  }

  QByteArray message((const char*)data + RINGLOG_TIMESTAMP_SIZE,
                     RINGLOG_MESSAGE_SIZE);
  logdll.log() << QString::fromUtf8(message);
}

void WindowsTunnelService::processLogs() {
  /* On the first pass, scan all log messages. */
  if (m_logindex < 0) {
    m_logindex = nextLogIndex();
    processMessage(m_logindex);
    m_logindex = (m_logindex + 1) % RINGLOG_MAX_ENTRIES;
  }

  /* Report new messages. */
  while (m_logindex != nextLogIndex()) {
    processMessage(m_logindex);
    m_logindex = (m_logindex + 1) % RINGLOG_MAX_ENTRIES;
  }
}

bool WindowsTunnelService::registerTunnelService(const QString& configFile) {
  logger.log() << "Register tunnel service";

  SC_HANDLE scm = (SC_HANDLE)m_scm;
  SC_HANDLE service = nullptr;
  auto guard = qScopeGuard([&] {
    if (service) {
      CloseServiceHandle(service);
    }
  });

  // Let's see if we have to delete a previous instance.
  service = OpenService(scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
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

  logger.log() << "The tunnel service exited with status code:"
               << status.dwWin32ExitCode << "-"
               << exitCodeToFailure(status.dwWin32ExitCode);

  emit backendFailure();
  return false;
}

static bool stopAndDeleteTunnelService(SC_HANDLE service) {
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

QString WindowsTunnelService::uapiCommand(const QString& command) {
  // Create a pipe to the tunnel service.
  LPTSTR tunnelName = (LPTSTR)TEXT(TUNNEL_NAMED_PIPE);
  HANDLE pipe = CreateFile(tunnelName, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           OPEN_EXISTING, 0, nullptr);
  if (pipe == INVALID_HANDLE_VALUE) {
    return QString();
  }

  auto guard = qScopeGuard([&] { CloseHandle(pipe); });
  if (!WaitNamedPipe(tunnelName, 1000)) {
    WindowsCommons::windowsLog("Failed to wait for named pipes");
    return QString();
  }

  DWORD mode = PIPE_READMODE_BYTE;
  if (!SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr)) {
    WindowsCommons::windowsLog("Failed to set the read-mode on pipe");
    return QString();
  }

  // Write the UAPI command into the pipe.
  QByteArray message = command.toLocal8Bit();
  DWORD written;
  while (!message.endsWith("\n\n")) {
    message.append('\n');
  }
  if (!WriteFile(pipe, message.constData(), message.length(), &written,
                 nullptr)) {
    WindowsCommons::windowsLog("Failed to write into the pipe");
    return QString();
  }

  // Receive the response from the pipe.
  QByteArray reply;
  while (!reply.contains("\n\n")) {
    char buffer[512];
    DWORD read = 0;
    if (!ReadFile(pipe, buffer, sizeof(buffer), &read, nullptr)) {
      break;
    }

    reply.append(buffer, read);
  }

  return QString::fromUtf8(reply).trimmed();
}

// static
static bool waitForServiceStatus(SC_HANDLE service, DWORD expectedStatus) {
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

// static
QString WindowsTunnelService::exitCodeToFailure(unsigned int exitCode) {
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