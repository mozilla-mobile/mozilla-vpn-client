/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKMANAGERCONTROLLER_H
#define NETWORKMANAGERCONTROLLER_H

#include <QDBusObjectPath>
#include <QObject>
#include <QVariant>
#include <QVersionNumber>

#include "controllerimpl.h"

class NetmgrDevice;
class QDateTime;
class QDBusInterface;
class QDBusError;

class NetmgrController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(NetmgrController)
  Q_OBJECT

 public:
  NetmgrController();
  ~NetmgrController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config,
                Controller::Reason reason) override;

  void deactivate(Controller::Reason reason) override;

  void checkStatus() override;


  enum Flags {
    TO_DISK = 0x01,
    IN_MEMORY = 0x02,
    IN_MEMORY_DETACHED = 0x04,
    IN_MEMORY_ONLY = 0x08,
    VOLATILE = 0x10,
    BLOCK_AUTOCONNECT = 0x20,
    NO_REAPPLY = 0x40,
  };
  Q_ENUM(Flags);

 private slots:
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

  static QDateTime guessTimestamp();

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


  QString m_connectionPath;
  NetmgrDevice* m_device = nullptr;
};

#endif  // NETWORKMANAGERCONTROLLER_H
