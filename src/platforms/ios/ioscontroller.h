/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSCONTROLLER_H
#define IOSCONTROLLER_H

#include <QObject>

#include "controllerimpl.h"

class IOSController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(IOSController)

 public:
  IOSController();
  ~IOSController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;

  void deleteOSTunnelConfig() override;

  void checkStatus() override;

  void forceDaemonSilentServerSwitch() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

  bool silentServerSwitchingSupported() const override { return false; }

 private:
  bool m_checkingStatus = false;
  QString m_serverPublicKey;
};

#endif  // IOSCONTROLLER_H
