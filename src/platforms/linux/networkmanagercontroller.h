/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKMANAGERCONTROLLER_H
#define NETWORKMANAGERCONTROLLER_H

#include <QObject>
#include <QVariant>
#include <QVersionNumber>

#include "controllerimpl.h"

class NetMgrConnection;
class QDBusConnection;
class QDBusInterface;
class QDBusObjectPath;
class QDBusError;

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

 private slots:
  void stateChanged(uint state, uint reason);
  void propertyChanged(QString interface, QVariantMap props, QStringList list);

  void initCompleted(const QDBusObjectPath& path, const QVariantMap& results);
  void peerCompleted(const QVariantMap& results);
  void activateCompleted(const QDBusObjectPath& path);
  void dbusError(const QDBusError& error);
  void dbusIgnoreError(const QDBusError& error);

  void deviceAdded(const QDBusObjectPath& path);
  void deviceRemoved(const QDBusObjectPath& path);
  void deviceStateChanged(uint state, uint prev, uint reason);

 private:
  static QVariantMap wgPeer(const InterfaceConfig& config);
  static void setDnsConfig(QVariantMap& map, const QHostAddress& server);
  static uint64_t readSysfsFile(const QString& path);
  void setActiveConnection(const QString& path);
  QVariant serializeConfig() const;

  static QString nmInterface(const QString& name);

 private:
  QVariantMap m_config;
  QVariantMap m_ipv4config;
  QVariantMap m_ipv6config;
  QVariantMap m_wireguard;

  QString m_serverPublicKey;
  QString m_serverIpv4Gateway;
  QString m_deviceIpv4Address;
  QString m_tunnelUuid;

  QVersionNumber m_version;
  QDBusInterface* m_client = nullptr;
  QDBusInterface* m_settings = nullptr;
  QDBusInterface* m_remote = nullptr;

  NetMgrConnection* m_connection = nullptr;
};

#endif  // NETWORKMANAGERCONTROLLER_H
