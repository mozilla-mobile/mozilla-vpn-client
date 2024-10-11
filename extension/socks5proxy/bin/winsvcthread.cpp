/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "winsvcthread.h"

#include <QCoreApplication>
#include <QDebug>

#include <windows.h>
#include <winsvc.h>

// static
bool WinSvcThread::startDispatcher(const QString& name) {
  auto serviceMain = [](DWORD argc, LPWSTR* argv) {
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    auto* svc = new WinSvcThread(QString::fromWCharArray(argv[0]));
    svc->start();
 };

  SERVICE_TABLE_ENTRYW serviceTable[] = {
    {(LPWSTR)name.utf16(), serviceMain},
    {nullptr, nullptr},
  };
  return StartServiceCtrlDispatcherW(serviceTable);
}

WinSvcThread::WinSvcThread(const QString& name, QObject* parent)
    : QThread(parent), m_serviceName(name) {
  m_serviceStatus = new SERVICE_STATUS;
  ZeroMemory(m_serviceStatus, sizeof(SERVICE_STATUS));
  m_serviceStatus->dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_serviceStatus->dwCurrentState = SERVICE_START_PENDING;
}

WinSvcThread::~WinSvcThread() {
  if (m_svcCtrlHandle) {
    m_serviceStatus->dwControlsAccepted = 0;
    m_serviceStatus->dwCurrentState = SERVICE_STOPPED;
    m_serviceStatus->dwCheckPoint = 1;
    SetServiceStatus(m_svcCtrlHandle, m_serviceStatus);
  }
  delete m_serviceStatus;
}

void WinSvcThread::run() {
  LPCWSTR wname = (LPCWSTR)m_serviceName.utf16();
  m_svcCtrlHandle = RegisterServiceCtrlHandlerEx(wname, svcCtrlHandler, this);
  if (!m_svcCtrlHandle) {
    qWarning() << "Failed to register the service handler";
    return;
  }
  if (!SetServiceStatus(m_svcCtrlHandle, m_serviceStatus)) {
    m_serviceStatus->dwWin32ExitCode = GetLastError();
    qWarning() << "SetServiceStatus failed";
    return;
  }

  if (!SetServiceStatus(m_svcCtrlHandle, m_serviceStatus)) {
    m_serviceStatus->dwWin32ExitCode = GetLastError();
    qWarning() << "SetServiceStatus failed";
    return;
  }

  // Set the service as running.
  m_serviceStatus->dwControlsAccepted = SERVICE_ACCEPT_STOP;
  m_serviceStatus->dwCurrentState = SERVICE_RUNNING;
  m_serviceStatus->dwWin32ExitCode = 0;
  m_serviceStatus->dwCheckPoint = 0;
  if (SetServiceStatus(m_svcCtrlHandle, m_serviceStatus) == FALSE) {
    qWarning() << "SetServiceStatus failed";
    return;
  }

  // Run the thread's event loop until the service shuts down.
  exec();

  // When the event loop exits, we are shutting down.
  m_serviceStatus->dwControlsAccepted = 0;
  m_serviceStatus->dwCurrentState = SERVICE_STOP_PENDING;
  m_serviceStatus->dwWin32ExitCode = 0;
  m_serviceStatus->dwCheckPoint = 4;
  if (SetServiceStatus(m_svcCtrlHandle, m_serviceStatus) == FALSE) {
    qWarning() << "SetServiceStatus failed";
  }

  // Request the application to exit.
  QCoreApplication::instance()->exit();
}

ulong WinSvcThread::svcCtrlHandler(ulong code, ulong type, void* evdata,
                                   void* context) {
  Q_UNUSED(type);
  Q_UNUSED(evdata);

  WinSvcThread* svc = reinterpret_cast<WinSvcThread*>(context);
  switch (code) {
    case SERVICE_CONTROL_STOP:
      svc->exit();
      return NO_ERROR;

    case SERVICE_CONTROL_INTERROGATE:
      // Always report that we support interrogation.
      return NO_ERROR;
    
    default:
      return ERROR_CALL_NOT_IMPLEMENTED;
  }
}
