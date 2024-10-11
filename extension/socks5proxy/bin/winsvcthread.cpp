/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "winsvcthread.h"

#include <QCoreApplication>
#include <QDebug>

#include <windows.h>
#include <winsvc.h>

// static
WinSvcThread* WinSvcThread::s_instance = nullptr;

WinSvcThread::WinSvcThread(const QString& name, QObject* parent)
    : QThread(parent), m_serviceName(name) {
  Q_ASSERT(s_instance == nullptr);
  s_instance = this;

  m_serviceStatus = new SERVICE_STATUS;
  ZeroMemory(m_serviceStatus, sizeof(SERVICE_STATUS));
  m_serviceStatus->dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_serviceStatus->dwCurrentState = SERVICE_START_PENDING;

  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this,
          &WinSvcThread::aboutToQuit);
}

void WinSvcThread::run() {
  auto lambdaServiceMain = [](DWORD argc, LPWSTR* argv) {
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    QStringList arguments;
    for (DWORD i = 0; i < argc; i++) {
      arguments.append(QString::fromWCharArray(argv[i]));
    }
    s_instance->svcMain(arguments);
  };

  SERVICE_TABLE_ENTRYW serviceTable[] = {
    {(LPWSTR)s_instance->m_serviceName.utf16(), lambdaServiceMain},
    {nullptr, nullptr},
  };

  // The service dispatcher blocks until the service is stopped.
  StartServiceCtrlDispatcherW(serviceTable);
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

void WinSvcThread::svcMain(const QStringList& arguments) {
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

  // Set the service as running.
  m_serviceStatus->dwControlsAccepted = SERVICE_ACCEPT_STOP;
  m_serviceStatus->dwCurrentState = SERVICE_RUNNING;
  m_serviceStatus->dwWin32ExitCode = 0;
  m_serviceStatus->dwCheckPoint = 0;
  if (SetServiceStatus(m_svcCtrlHandle, m_serviceStatus) == FALSE) {
    qWarning() << "SetServiceStatus failed";
    return;
  }
}

void WinSvcThread::aboutToQuit() {
  m_serviceStatus->dwControlsAccepted = 0;
  m_serviceStatus->dwCurrentState = SERVICE_STOPPED;
  m_serviceStatus->dwWin32ExitCode = GetLastError();
  m_serviceStatus->dwCheckPoint = 1;
  if (SetServiceStatus(m_svcCtrlHandle, m_serviceStatus) == FALSE) {
    qWarning() << "SetServiceStatus failed";
  }
}

void WinSvcThread::svcCtrlStop() {
  if (m_serviceStatus->dwCurrentState != SERVICE_RUNNING) {
    return;
  }
  m_serviceStatus->dwControlsAccepted = 0;
  m_serviceStatus->dwCurrentState = SERVICE_STOP_PENDING;
  m_serviceStatus->dwWin32ExitCode = 0;
  m_serviceStatus->dwCheckPoint = 4;
  if (SetServiceStatus(m_svcCtrlHandle, m_serviceStatus) == FALSE) {
    qWarning() << "SetServiceStatus failed";
  }

  // Request the application to exit.
  QCoreApplication::instance()->quit();
}

ulong WinSvcThread::svcCtrlHandler(ulong code, ulong type, void* evdata,
                                   void* context) {
  Q_UNUSED(type);
  Q_UNUSED(evdata);

  WinSvcThread* svc = reinterpret_cast<WinSvcThread*>(context);
  switch (code) {
    case SERVICE_CONTROL_STOP:
      svc->svcCtrlStop();
      return NO_ERROR;

    case SERVICE_CONTROL_INTERROGATE:
      // Always report that we support interrogation.
      return NO_ERROR;
    
    default:
      return ERROR_CALL_NOT_IMPLEMENTED;
  }
}
