/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXCONTROLLER_H
#define LINUXCONTROLLER_H

#include "controllerimpl.h"

#include <QObject>

class DBusClient;
class QDBusPendingCallWatcher;

class LinuxController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LinuxController)

 public:
  LinuxController();
  ~LinuxController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const Server& server, const Device* device, const Keys* keys,
                const QList<IPAddressRange>& allowedIPAddressRanges,
                const QList<QString>& vpnDisabledApps,
                bool forSwitching) override;

  void deactivate(bool forSwitching) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private slots:
  void checkStatusCompleted(QDBusPendingCallWatcher* call);
  void initializeCompleted(QDBusPendingCallWatcher* call);
  void operationCompleted(QDBusPendingCallWatcher* call);

 private:
  DBusClient* m_dbus = nullptr;
};

#endif  // LINUXCONTROLLER_H
