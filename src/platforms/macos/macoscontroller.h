/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSCONTROLLER_H
#define MACOSCONTROLLER_H

#include "controllerimpl.h"
#include "localsocketcontroller.h"

class MacOSController final : public LocalSocketController {
  Q_DISABLE_COPY_MOVE(MacOSController)

 public:
  MacOSController();
  ~MacOSController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;;

  void checkStatus() override;

  void getBackendLogs(QIODevice* device) override;

  void cleanupBackendLogs() override;

 private slots:
  void registerService();

 private:
  NSString* plist() const;

  bool m_permissionRequired = false;
  QTimer m_regTimer;

  // NSXPCConnection to the daemon.
  void* m_connection = nullptr;
};

#endif  // MACOSCONTROLLER_H
