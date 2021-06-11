/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsservicemanager.h"

#include "logger.h"
#include "mozillavpn.h"
#include "windowscommons.h"

#include <QTimer>
#include <Windows.h>
#include <Winsvc.h>

namespace {
Logger logger(LOG_WINDOWS, "WindowsServiceManager");
constexpr int POLL_TIME = 500;
}  // namespace

WindowsServiceManager::WindowsServiceManager() {
  DWORD err = NULL;
  auto scm_rights = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE |
                    SC_MANAGER_QUERY_LOCK_STATUS | STANDARD_RIGHTS_READ;
  m_serviceManager = OpenSCManager(NULL,  // local computer
                                   NULL,  // servicesActive database
                                   scm_rights);
  err = GetLastError();
  if (err != NULL) {
    WindowsCommons::windowsLog("OpenSCManager failed");
    return;
  }
  logger.log() << "OpenSCManager access given";

  // Try to get an elevated handle
  m_service = OpenService(m_serviceManager,  // SCM database
                          m_serviceName,     // name of service
                          (GENERIC_READ | SERVICE_START));
  if (m_service == NULL) {
    WindowsCommons::windowsLog("OpenService failed");
    return;
  }
  m_timer.setSingleShot(false);
  connect(&m_timer, &QTimer::timeout, this, &WindowsServiceManager::pollStatus);

  logger.log() << "Service access granted";
}

WindowsServiceManager::~WindowsServiceManager() {
  if (m_service != NULL) {
    CloseServiceHandle(m_service);
  }
  if (m_serviceManager != NULL) {
    CloseServiceHandle(m_serviceManager);
  }
}

void WindowsServiceManager::pollStatus() {
  if (m_service == NULL) {
    logger.log() << "Need read access to poll service state";
    return;
  }
  auto state = getStatus();
  logger.log() << "Polling Status" << m_goalState << "wanted, has: " << state;
  if ((state != m_goalState)) {
    if (m_currentWaitTime >= m_maxWaitTime) {
      m_timer.stop();
      logger.log() << "Waiting for Service failed";
      MozillaVPN::instance()->errorHandle(ErrorHandler::ControllerError);
      return;
    }
    m_currentWaitTime++;
    return;
  }
  m_timer.stop();
  if (state == SERVICE_RUNNING) {
    emit serviceStarted();
    return;
  }
}

void WindowsServiceManager::startPolling(DWORD goalState, int maxWaitSec) {
  m_goalState = goalState;
  m_maxWaitTime = maxWaitSec;
  m_currentWaitTime = 0;
  m_timer.start(POLL_TIME);
}

DWORD WindowsServiceManager::getStatus() const {
  SERVICE_STATUS_PROCESS serviceStatus;
  if (m_service == NULL) {
    logger.log() << "Need access to get service state";
    return SERVICE_STOPPED;
  }
  DWORD dwBytesNeeded;  // Contains missing bytes if struct is too small?
  bool ok =
      QueryServiceStatusEx(m_service,               // handle to service
                           SC_STATUS_PROCESS_INFO,  // information level
                           (LPBYTE)&serviceStatus,  // address of structure
                           sizeof(SERVICE_STATUS_PROCESS),  // size of structure
                           &dwBytesNeeded);
  if (ok) {
    return serviceStatus.dwCurrentState;
  }
  return SERVICE_STOPPED;
}

bool WindowsServiceManager::startService() {
  if (m_service == NULL) {
    logger.log() << "Need access to start service";
    return false;
  }
  auto state = getStatus();
  if (state != SERVICE_STOPPED && state != SERVICE_STOP_PENDING) {
    logger.log() << ("Service start not possible, as its running");
    emit serviceStarted();
    return true;
  }
  // In case he have execute rights, lets boot and wait for the service.
  bool ok = StartService(m_service,  // handle to service
                         0,          // number of arguments
                         NULL);      // no arguments
  if (ok) {
    logger.log() << ("Service start requested");
    startPolling(SERVICE_RUNNING, 10);
  } else {
    WindowsCommons::windowsLog("StartService failed");
  }
  return ok;
}
