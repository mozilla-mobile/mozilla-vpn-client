/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemonserver.h"

#include <AclAPI.h>
#include <Windows.h>
#include <winsvc.h>

#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QThread>
#include <QTimer>

#include "commandlineparser.h"
#include "constants.h"
#include "daemon/daemonlocalserverconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "version.h"
#include "windowsdaemon.h"

#define SERVICE_NAME (wchar_t*)L"Mozilla  VPN"

namespace {
Logger logger("WindowsDaemonServer");

SERVICE_STATUS s_serviceStatus = {0};
SERVICE_STATUS_HANDLE s_statusHandle = nullptr;
HANDLE s_serviceStopEvent = INVALID_HANDLE_VALUE;
}  // namespace

class ServiceThread : public QThread {
 private:
  void run() override {
    SERVICE_TABLE_ENTRY serviceTable[] = {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)serviceMain},
        {nullptr, nullptr},
    };

    if (StartServiceCtrlDispatcher(serviceTable) == FALSE) {
      logger.error() << "Failed to start the service.";
    }
  }

  static void WINAPI serviceMain(DWORD argc, wchar_t** argv);
  static void WINAPI serviceCtrlHandler(DWORD code);
};

WindowsDaemonServer::WindowsDaemonServer(QObject* parent)
    : Command(parent, "windowsdaemon", "Activate the windows daemon") {
  MZ_COUNT_CTOR(WindowsDaemonServer);
}

WindowsDaemonServer::~WindowsDaemonServer() {
  MZ_COUNT_DTOR(WindowsDaemonServer);
}

int WindowsDaemonServer::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  logger.debug() << "Daemon is starting";

  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN Daemon");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  if (tokens.length() > 1) {
    QList<CommandLineParser::Option*> options;
    return CommandLineParser::unknownOption(this, appName, tokens[1], options,
                                            false);
  }

  ServiceThread* st = new ServiceThread();
  st->start();

  WindowsDaemon daemon;

  QLocalServer server(qApp);
  server.setSocketOptions(QLocalServer::WorldAccessOption);
  connect(&server, &QLocalServer::newConnection, [&]() {
    logger.debug() << "New connection received";
    if (!server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = server.nextPendingConnection();
    Q_ASSERT(socket);

#ifndef MZ_DEBUG
    auto check = canConnect(socket);
    if (!check.canConnect) {
      logger.error() << "Rejecting connection:" << check.rejectionReason;
      socket->disconnectFromServer();
      socket->deleteLater();
      return;
    }
#endif

    new DaemonLocalServerConnection(&daemon, socket);
  });
  if (!server.listen(Constants::WINDOWS_DAEMON_PATH)) {
    logger.error() << "Failed to initialize the server";
    return 1;
  }

  QTimer stopEventTimer;
  connect(&stopEventTimer, &QTimer::timeout, [&]() {
    if (WaitForSingleObject(s_serviceStopEvent, 0) == WAIT_OBJECT_0) {
      logger.debug() << "Stop event message received";

      s_serviceStatus.dwControlsAccepted = 0;
      s_serviceStatus.dwCurrentState = SERVICE_STOPPED;
      s_serviceStatus.dwWin32ExitCode = GetLastError();
      s_serviceStatus.dwCheckPoint = 1;

      if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE) {
        logger.error() << "SetServiceStatus failed";
      }
      CloseHandle(s_serviceStopEvent);

      qApp->exit();
    }
  });
  stopEventTimer.start(1000);

  return qApp->exec();
}

// static
void WINAPI ServiceThread::serviceMain(DWORD argc, LPTSTR* argv) {
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  logger.debug() << "Service has started";

  s_statusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, serviceCtrlHandler);
  if (!s_statusHandle) {
    logger.error() << "Failed to register the service handler";
    return;
  }

  ZeroMemory(&s_serviceStatus, sizeof(s_serviceStatus));
  s_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  s_serviceStatus.dwCurrentState = SERVICE_START_PENDING;

  if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE) {
    logger.error() << "SetServiceStatus failed";
  }

  s_serviceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!s_serviceStopEvent) {
    logger.error() << "Failed to create the stop event";

    s_serviceStatus.dwControlsAccepted = 0;
    s_serviceStatus.dwCurrentState = SERVICE_STOPPED;
    s_serviceStatus.dwWin32ExitCode = GetLastError();
    s_serviceStatus.dwCheckPoint = 1;

    if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE) {
      logger.error() << "SeServiceStatus failed";
    }

    return;
  }

  s_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  s_serviceStatus.dwCurrentState = SERVICE_RUNNING;
  s_serviceStatus.dwWin32ExitCode = 0;
  s_serviceStatus.dwCheckPoint = 0;

  if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE) {
    logger.error() << "SeServiceStatus failed";
    return;
  }

  // Let's this thread die.
}

// static
void WINAPI ServiceThread::serviceCtrlHandler(DWORD code) {
  logger.debug() << "Message received";

  if (code != SERVICE_CONTROL_STOP) {
    return;
  }

  if (s_serviceStatus.dwCurrentState != SERVICE_RUNNING) {
    return;
  }

  s_serviceStatus.dwControlsAccepted = 0;
  s_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
  s_serviceStatus.dwWin32ExitCode = 0;
  s_serviceStatus.dwCheckPoint = 4;

  if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE) {
    logger.error() << "SetServiceStatus failed";
  }

  SetEvent(s_serviceStopEvent);
}
// returns whether the provided file information is valid
static inline bool isValidFileInfo(const BY_HANDLE_FILE_INFORMATION& fi) {
  return fi.dwVolumeSerialNumber != 0 || fi.nFileIndexHigh != 0 ||
         fi.nFileIndexLow != 0;
}
// Opens the Current Process and returns its File Informaton
static BY_HANDLE_FILE_INFORMATION resolveSelf() {
  BY_HANDLE_FILE_INFORMATION out{};
  ZeroMemory(&out, sizeof(out));

  DWORD cap = 32768;
  std::unique_ptr<wchar_t[]> buf(new wchar_t[cap]);
  DWORD len = ::GetModuleFileNameW(nullptr, buf.get(), cap);
  if (len == 0 || len >= cap) return out;

  HANDLE hSelf =
      ::CreateFileW(buf.get(),
                    0,  // query-only
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (!hSelf || hSelf == INVALID_HANDLE_VALUE) return out;
  auto selfGuard = qScopeGuard([&] { ::CloseHandle(hSelf); });

  if (!::GetFileInformationByHandle(hSelf, &out)) {
    BY_HANDLE_FILE_INFORMATION empty{};  // ensure zero on failure
    ZeroMemory(&empty, sizeof(empty));
    return empty;
  }
  return out;
}

// Returns the File Information of the executable of the provided QLocalSocket
// client
static BY_HANDLE_FILE_INFORMATION resolveClient(QLocalSocket* sock) {
  BY_HANDLE_FILE_INFORMATION out{};
  ZeroMemory(&out, sizeof(out));

  if (!sock || sock->state() != QLocalSocket::ConnectedState) return out;

  qintptr sd = sock->socketDescriptor();
  if (sd == -1) return out;
  HANDLE pipeHandle = reinterpret_cast<HANDLE>(sd);

  ULONG clientPid = 0;
  if (!::GetNamedPipeClientProcessId(pipeHandle, &clientPid) || clientPid == 0)
    return out;

  HANDLE hProc =
      ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, clientPid);
  if (!hProc) return out;
  auto procGuard = qScopeGuard([&] { ::CloseHandle(hProc); });

  DWORD cap = 32768;
  std::unique_ptr<wchar_t[]> buf(new wchar_t[cap]);
  DWORD len = cap;
  if (!::QueryFullProcessImageNameW(hProc, 0, buf.get(), &len)) return out;

  HANDLE clientFile =
      ::CreateFileW(buf.get(),
                    0,  // query-only
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (!clientFile || clientFile == INVALID_HANDLE_VALUE) return out;
  auto clientGuard = qScopeGuard([&] { ::CloseHandle(clientFile); });

  if (!::GetFileInformationByHandle(clientFile, &out)) {
    BY_HANDLE_FILE_INFORMATION empty{};
    ZeroMemory(&empty, sizeof(empty));
    return empty;
  }
  return out;
}

// Given two file informations, returns whether they point to the same file
static inline bool isSameExecutable(const BY_HANDLE_FILE_INFORMATION& a,
                                    const BY_HANDLE_FILE_INFORMATION& b) {
  return a.dwVolumeSerialNumber == b.dwVolumeSerialNumber &&
         a.nFileIndexHigh == b.nFileIndexHigh &&
         a.nFileIndexLow == b.nFileIndexLow;
}

WindowsDaemonServer::CheckResult WindowsDaemonServer::canConnect(
    QLocalSocket* sock) {
  if (!sock) return {false, QStringLiteral("Socket pointer is null.")};

  const auto selfInfo = resolveSelf();
  if (!isValidFileInfo(selfInfo))
    return {false,
            QStringLiteral("Failed to resolve current executable identity.")};

  const auto clientInfo = resolveClient(sock);
  if (!isValidFileInfo(clientInfo))
    return {false,
            QStringLiteral("Failed to resolve client executable identity.")};

  if (!isSameExecutable(clientInfo, selfInfo))
    return {false,
            QStringLiteral("Caller executable does not match current binary.")};

  return {true, QString()};
}

static Command::RegistrationProxy<WindowsDaemonServer> s_commandWindowsDaemon;
