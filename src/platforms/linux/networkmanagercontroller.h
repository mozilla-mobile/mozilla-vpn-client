/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKMANAGERCONTROLLER_H
#define NETWORKMANAGERCONTROLLER_H

#include <QObject>

#include "controllerimpl.h"

struct _GAsyncResult;
struct _GObject;
struct _GCancellable;
struct _NMActiveConnection;
struct _NMCLient;
struct _NMRemoteConnection;
struct _NMSetting;

class NetworkManagerController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(NetworkManagerController)
  Q_OBJECT

 public:
  NetworkManagerController();
  ~NetworkManagerController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private slots:
  void initializeCompleted(void*);
  void peerConfigCompleted(void*);
  void activateCompleted(void*);
  void deactivateCompleted(void*);

 private:
  static uint64_t readSysfsFile(const QString& path); 

 private:
  struct _GCancellable* m_cancellable;
  struct _NMClient* m_client = nullptr;
  struct _NMSetting* m_wireguard = nullptr;
  struct _NMSetting* m_ipv4config = nullptr;
  struct _NMSetting* m_ipv6config = nullptr;
  struct _NMRemoteConnection* m_remote = nullptr;
  struct _NMActiveConnection* m_active = nullptr;

  QString m_serverPublicKey;
  QString m_serverIpv4Gateway;
  QString m_tunnelUuid;
};

#endif  // NETWORKMANAGERCONTROLLER_H
