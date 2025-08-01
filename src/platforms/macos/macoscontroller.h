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

  void deactivate() override;

  void checkStatus() override;

  void getBackendLogs(QIODevice* device) override;

  void cleanupBackendLogs() override;

  void forceDaemonCrash() override;

 private slots:
  void upgradeService();
  void registerService();
  void connectService();

 private:
  NSString* plist() const;
  NSString* machServiceName() const;

 private:
  QString plistName() const;
#ifdef __OBJC__
  NSObject<XpcDaemonProtocol>* remoteObject();
#endif

  QTimer m_registerTimer;
  QTimer m_connectTimer;
  bool m_permissionRequired = false;

  // NSXPCConnection to the daemon.
  void* m_connection = nullptr;
};

#endif  // MACOSCONTROLLER_H
