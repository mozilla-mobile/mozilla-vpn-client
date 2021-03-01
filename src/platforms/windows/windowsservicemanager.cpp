#include "logger.h"
#include "mozillavpn.h"
#include <Windows.h>
#include "Windows.h"
#include "Winsvc.h"
#include "windowsservicemanager.h"
#include "windowscommons.h"
#include <QTimer>

namespace {
Logger logger(LOG_WINDOWS, "WindowsServiceManager");
}

WindowsServiceManager::WindowsServiceManager() {
  DWORD err = NULL;
  auto scm_rights = SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE |
                    SC_MANAGER_QUERY_LOCK_STATUS | STANDARD_RIGHTS_READ;
  m_serviceManager = OpenSCManager(NULL,  // local computer
                                   NULL,  // servicesActive database
                                   scm_rights);
  err = GetLastError();
  if (err != NULL) {
    logger.log() << " OpenSCManager failed code: " << err;
    return;
  }
  logger.log() << "OpenSCManager access given - " << err;

  // Try to get an elevated handle
  m_service = OpenService(m_serviceManager,  // SCM database
                          m_serviceName,     // name of service
                          (GENERIC_READ | SERVICE_START | SERVICE_STOP));
  err = GetLastError();
  if (err != NULL) {
    WindowsCommons::windowsLog("OpenService failed");
    return;
  }
  m_has_access = true;
  m_timer.setSingleShot(false);
  connect(&m_timer, &QTimer::timeout, this, &WindowsServiceManager::pollStatus);

  logger.log() << "Service manager execute access granted";
}

WindowsServiceManager::~WindowsServiceManager() {
  if (m_service != NULL) {
    CloseServiceHandle(m_service);
  }
  if (m_serviceManager != NULL) {
    CloseServiceHandle(m_serviceManager);
  }
}

void WindowsServiceManager::startPolling(DWORD goal_state, int max_wait_sec) {
  m_state_target = goal_state;
  m_maxWaitTime = max_wait_sec;
  m_currentWaitTime = 0;
  m_timer.start(1000);
}

void WindowsServiceManager::pollStatus() {
  if (!m_has_access) {
    logger.log() << "Need read access to poll service state";
    return;
  }
  auto state = getStatus().dwCurrentState;
  logger.log() << "Polling Status" << m_state_target
               << "wanted, has: " << state;
  if ((state != m_state_target)) {
    if (m_currentWaitTime >= m_maxWaitTime) {
      m_timer.stop();
      logger.log() << "Waiting for Service failed";
      MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
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
  if (state == SERVICE_STOPPED) {
    emit serviceStopped();
    return;
  }
}

SERVICE_STATUS_PROCESS WindowsServiceManager::getStatus() {
  SERVICE_STATUS_PROCESS serviceStatus;
  if (!m_has_access) {
    logger.log() << "Need read access to get service state";
    return serviceStatus;
  }
  DWORD dwBytesNeeded;  // Contains missing bytes if struct is too small?
  QueryServiceStatusEx(m_service,                       // handle to service
                       SC_STATUS_PROCESS_INFO,          // information level
                       (LPBYTE)&serviceStatus,          // address of structure
                       sizeof(SERVICE_STATUS_PROCESS),  // size of structure
                       &dwBytesNeeded);
  return serviceStatus;
}

bool WindowsServiceManager::startService() {
  auto state = getStatus().dwCurrentState;
  if (state != SERVICE_STOPPED && state != SERVICE_STOP_PENDING) {
    logger.log() << ("Service start not possible, as its running");
    emit serviceStarted();
    return true;
  }
  if (!m_has_access) {
    logger.log() << "Need execute access to start service";
    // Still start polling - windows should start the service
    m_state_target = SERVICE_RUNNING;
    pollStatus();
    return false;
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
    MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
  }
  return ok;
}

bool WindowsServiceManager::stopService() {
  if (!m_has_access) {
    logger.log() << "Need execute access to stop services";
    return false;
  }
  auto state = getStatus().dwCurrentState;
  if (state != SERVICE_RUNNING && state != SERVICE_START_PENDING) {
    logger.log() << ("Service stop not possible, as its not running");
  }

  bool ok = ControlService(m_service, SERVICE_CONTROL_STOP, NULL);
  if (ok) {
    logger.log() << ("Service stop requested");
    startPolling(SERVICE_STOPPED, 10);
  } else {
    WindowsCommons::windowsLog("StopService failed");
    MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
  }
  return ok;
}
