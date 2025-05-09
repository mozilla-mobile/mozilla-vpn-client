/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSCONTROLLER_H
#define MACOSCONTROLLER_H

#include "controllerimpl.h"

#ifdef __OBJC__
#  include "xpcdaemonprotocol.h"
#endif

class MacOSController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(MacOSController)

 public:
  MacOSController();
  ~MacOSController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config, Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(QObject* receiver, const char* method) override;

  void cleanupBackendLogs() override;

  void forceDaemonCrash() override;

 private slots:
  void checkInitialization();
  void upgradeInitialization();

 private:
  QString plistName() const;
#ifdef __OBJC__
  NSObject<XpcDaemonProtocol>* remoteObject();
#endif

  QTimer m_initTimer;

  // NSXPCConnection to the daemon.
  void* m_connection = nullptr;
};

#endif  // MACOSCONTROLLER_H
