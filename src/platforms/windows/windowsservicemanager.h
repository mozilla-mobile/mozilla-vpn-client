/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSSERVICEMANAGER_H
#define WINDOWSSERVICEMANAGER_H

#include <QTimer>
#include <QObject>

#include <Windows.h>
#include <Winsvc.h>

/**
 * @brief The WindowsServiceManager provides controll over the MozillaVPNBroker
 * service via SCM
 */
class WindowsServiceManager final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsServiceManager)

 public:
  WindowsServiceManager();
  ~WindowsServiceManager();

  // true if the Service is running
  bool isRunning() const { return getStatus() == SERVICE_RUNNING; };

  // Starts the service if execute rights are present
  // Starts to poll for serviceStarted
  bool startService();

  // Starts to poll for serviceStopped
  void pollStatus();

 signals:
  // Gets Emitted after the Service moved From SERVICE_START_PENDING to
  // SERVICE_RUNNING
  void serviceStarted();

 private:
  // Returns the State of the Process:
  // See
  // SERVICE_STOPPED,SERVICE_STOP_PENDING,SERVICE_START_PENDING,SERVICE_RUNNING
  DWORD getStatus() const;
  bool m_hasAccess = false;
  LPCWSTR m_serviceName = L"MozillaVPNBroker";
  SC_HANDLE m_serviceManager;
  SC_HANDLE m_service;  // Service handle with r/w priv.
  DWORD m_goalState;
  int m_currentWaitTime;
  int m_maxWaitTime;
  QTimer m_timer;

  void startPolling(DWORD goalState, int maxWaitSec);
};

#endif  // WINDOWSSERVICEMANAGER_H
