/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H

#include "controllerimpl.h"
#include "models/device.h"

class AndroidController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(AndroidController)

 public:
  AndroidController();
  static AndroidController* instance();
  ~AndroidController();

  // from ControllerImpl
  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                ConnectionManager::Reason Reason) override;

  void deactivate(ConnectionManager::Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  bool m_init = false;
  QString m_deviceName;
  QString m_devicePublicKey;
  qint64 m_deviceCreationTime;
  QString m_serverPublicKey;
  std::function<void(const QString&)> m_logCallback;
};

#endif  // ANDROIDCONTROLLER_H
