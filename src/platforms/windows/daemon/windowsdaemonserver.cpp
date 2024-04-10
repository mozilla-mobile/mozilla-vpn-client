/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemonserver.h"

#include <AclAPI.h>
#include <Windows.h>
#include <winsvc.h>

#include <QCoreApplication>
#include <QThread>
#include <QTimer>

#include "commandlineparser.h"
#include "constants.h"
#include "daemon/daemonlocalserver.h"
#include "logger.h"
#include "utils/leakdetector/leakdetector.h"
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
  QCoreApplication::setApplicationVersion(Constants::versionString());

  if (tokens.length() > 1) {
    QList<CommandLineParser::Option*> options;
    return CommandLineParser::unknownOption(this, appName, tokens[1], options,
                                            false);
  }

  ServiceThread* st = new ServiceThread();
  st->start();

  WindowsDaemon daemon;

  DaemonLocalServer server(qApp);
  if (!server.initialize()) {
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

static Command::RegistrationProxy<WindowsDaemonServer> s_commandWindowsDaemon;
