/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKMANAGERCONTROLLER_H
#define NETWORKMANAGERCONTROLLER_H

#include <QDBusObjectPath>
#include <QHostAddress>
#include <QObject>
#include <QVariant>
#include <QVersionNumber>
#include <memory>

#include "controllerimpl.h"

class NetmgrDevice;
class Obfuscator;
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

  void deactivate() override;

  void checkStatus() override;

  bool obfuscationSupported() const override;

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
  void deviceAdded(const QDBusObjectPath& path);
  void deviceRemoved(const QDBusObjectPath& path);
  void deviceStateChanged(uint state, uint prev, uint reason);

 private:
  enum class ProfileState {
    // No profile with our UUID exists in NetworkManager.
    Absent,
    // Deleting a stale profile left over from a previous session.
    Cleaning,
    // AddConnection2 is in progress.
    Adding,
    // The profile exists and m_remote points at it.
    Ready,
  };
  Q_ENUM(ProfileState)

  static QVariantMap wgPeer(const InterfaceConfig& config);
  static void setDnsConfig(QVariantMap& map, const QHostAddress& server);
  static uint64_t readSysfsFile(const QString& path);
  void setActiveConnection(const QString& path);
  void peerCompleted(const QVariantMap& results);
  void activateCompleted(const QDBusObjectPath& path);
  void dbusBackendError(const QDBusError& error);

  void setProfileState(ProfileState state);
  void advance();
  void addConnection();
  void updateConnection();
  QVariant serializeConfig() const;

  static QString nmInterface(const QString& name);

  static QDateTime guessUptime();

 private:
  QVariantMap m_config;
  QVariantMap m_ipv4config;
  QVariantMap m_ipv6config;
  QVariantMap m_wireguard;

  QString m_serverPublicKey;
  QHostAddress m_serverIpv4Gateway;
  QHostAddress m_serverIpv6Gateway;
  QHostAddress m_deviceIpv4Address;
  QHostAddress m_deviceIpv6Address;
  QString m_uuid;

  QVersionNumber m_version;
  QDBusInterface* m_client = nullptr;
  QDBusInterface* m_remote = nullptr;

  ProfileState m_profileState = ProfileState::Absent;

  // True between activate() and deactivate(). deactivate() can arrive while
  // activation requests are in progress and their replies must not bring the
  // tunnel up. Split from ProfileState because they change independently.
  bool m_wantActive = false;

  QString m_connectionPath;
  NetmgrDevice* m_device = nullptr;

  // Active connection that was taken down because its obfuscator relay could
  // not be restarted, its device events are ignored.
  QString m_relayFailedConnection;

  std::unique_ptr<Obfuscator> m_obfuscator;
};

#endif  // NETWORKMANAGERCONTROLLER_H
