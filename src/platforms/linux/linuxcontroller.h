/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXCONTROLLER_H
#define LINUXCONTROLLER_H

#include <QHostAddress>
#include <QObject>

#include "controllerimpl.h"

class DBusClient;
class QDBusPendingCallWatcher;

class LinuxController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LinuxController)

 public:
  LinuxController();
  ~LinuxController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

  bool multihopSupported() override { return true; }

 private slots:
  void checkStatusCompleted(QDBusPendingCallWatcher* call);
  void initializeCompleted(QDBusPendingCallWatcher* call);
  void operationCompleted(QDBusPendingCallWatcher* call);

 private:
  DBusClient* m_dbus = nullptr;
};

#endif  // LINUXCONTROLLER_H
