/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowstunnelmonitor.h"
#include "leakdetector.h"
#include "logger.h"
#include "windowsdaemon.h"
#include "windowscommons.h"

#include <QScopeGuard>

#include <Windows.h>

constexpr uint32_t WINDOWS_TUNNEL_MONITOR_TIMEOUT_MSEC = 2000;

namespace {
Logger logger(LOG_WINDOWS, "WindowsTunnelMonitor");
}

WindowsTunnelMonitor::WindowsTunnelMonitor() {
  MVPN_COUNT_CTOR(WindowsTunnelMonitor);

  connect(&m_timer, &QTimer::timeout, this, &WindowsTunnelMonitor::timeout);
}

WindowsTunnelMonitor::~WindowsTunnelMonitor() {
  MVPN_COUNT_CTOR(WindowsTunnelMonitor);
}

void WindowsTunnelMonitor::start() {
  logger.log() << "Starting monitoring the tunnel service";
  m_timer.start(WINDOWS_TUNNEL_MONITOR_TIMEOUT_MSEC);
}

void WindowsTunnelMonitor::stop() {
  logger.log() << "Stopping monitoring the tunnel service";
  m_timer.stop();
}

void WindowsTunnelMonitor::timeout() {
  logger.log() << "Checking the tunnel service state";

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
    emit backendFailure();
    return;
  }

  // Let's see if we have to delete a previous instance.
  service = OpenService(scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
  if (!service) {
    logger.log() << "The service doesn't exist";
    emit backendFailure();
    return;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    WindowsCommons::windowsLog("Failed to retrieve the service status");
    emit backendFailure();
    return;
  }

  if (status.dwCurrentState == SERVICE_RUNNING) {
    logger.log() << "The service is active";
    return;
  }

  emit backendFailure();
}
