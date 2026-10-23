/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "netmgrcontroller.h"

#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QFile>
#include <QScopeGuard>
#include <QUuid>

#include "daemon/obfuscator/qprocessobfuscator.h"
#include "dbustypes.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "netmgrdevice.h"
#include "netmgrtypes.h"
#include "settingsholder.h"

constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
constexpr const char* WG_INTERFACE_NAME = "wg0";
constexpr uint16_t WG_KEEPALIVE_PERIOD = 60;
constexpr uint16_t WG_EXCLUDE_RULE_PRIO = 100;
// NMDeviceStateReason: the device is deactivated because a new activation
// replaces the current one, e.g. activate() switching servers.
constexpr uint NM_DEVICE_STATE_REASON_NEW_ACTIVATION = 60;

namespace {
Logger logger("NetmgrController");
}

NetmgrController::NetmgrController() {
  MZ_COUNT_CTOR(NetmgrController);

  m_client =
      new QDBusInterface(DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE,
                         QDBusConnection::systemBus(), this);
  if (!m_client->isValid()) {
    logger.warning() << "NetworkManager is not available";
    return;
  }

  QVariant version = m_client->property("Version");
  m_version = QVersionNumber::fromString(version.toString());
  logger.info() << "NetworkManager version:" << m_version.toString();
}

NetmgrController::~NetmgrController() {
  MZ_COUNT_DTOR(NetmgrController);
  logger.debug() << "Destroying NetmgrController";

  // Stop the obfuscator relay first so its process doesn't outlive us.
  m_obfuscator.reset();

  // Remove the connection from NetworkManager
  if (m_remote) {
    QDBusReply<void> reply = m_remote->call("Delete");
    if (!reply.isValid()) {
      logger.warning() << "Failed to delete connection on shutdown:"
                       << reply.error().message();
    }
  }
}

bool NetmgrController::obfuscationSupported() const {
  // Obfuscation bypass routes use the "throw" route type, which needs
  // NetworkManager >= 1.38.
  return m_version >= QVersionNumber(1, 38);
}

// static
QString NetmgrController::nmInterface(const QString& name) {
  return QStringLiteral(DBUS_NM_INTERFACE) + "." + name;
}

void NetmgrController::initialize(const Device* device, const Keys* keys) {
  if (!m_client || !m_client->isValid()) {
    logger.warning() << "NetworkManager client is not available";
    // Not a permission problem, but the same semantics: blocked until the user
    // acts outside the app. Shows ViewNetworkManagerUnavailable.
    emit permissionRequired();
    return;
  }

  // Ensure we use a consistent UUID for the wireguard interface.
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  QUuid uuid(settingsHolder->tunnelUuid());
  if (uuid.isNull()) {
    uuid = QUuid::createUuid();
    settingsHolder->setTunnelUuid(uuid.toString(QUuid::WithoutBraces));
  }
  m_uuid = uuid.toString(QUuid::WithoutBraces);

  // Port 53 doesn't need the obfuscator relay, every other method does.
  if (!obfuscationSupported()) {
    if (settingsHolder->obfuscationPolicy() !=
            SettingsHolder::ObfuscationPolicy::NoObfuscation &&
        settingsHolder->obfuscationPolicy() !=
            SettingsHolder::ObfuscationPolicy::Port53) {
      logger.warning()
          << "NetworkManager" << m_version.toString()
          << "is too old for this obfuscation method, disabling it";
      settingsHolder->setObfuscationPolicy(
          SettingsHolder::ObfuscationPolicy::NoObfuscation);
    }
    settingsHolder->setLastObfuscationPolicy(
        SettingsHolder::ObfuscationPolicy::Port53);
  }

  m_deviceIpv4Address = QHostAddress(device->ipv4Address().split('/').first());
  m_deviceIpv6Address = QHostAddress(device->ipv6Address().split('/').first());

  // Generic connection settings.
  m_config.insert("id", QCoreApplication::applicationName());
  m_config.insert("interface-name", QString(WG_INTERFACE_NAME));
  m_config.insert("type", QString("wireguard"));
  m_config.insert("uuid", m_uuid);
  m_config.insert("autoconnect", false);
  // Make the connection private to the user, so NetworkManager checks
  // settings.modify.own instead of settings.modify.system. That is also
  // granted to inactive sessions, e.g. when reconnecting behind a lock screen.
  struct passwd* pw = getpwuid(getuid());
  if (pw) {
    m_config.insert("permissions",
                    QStringList{"user:" + QString::fromLocal8Bit(pw->pw_name)});
  } else {
    logger.warning() << "Unable to determine the user name";
  }
  // TODO: Timestamp?

  QStringList ipv4split = device->ipv4Address().split('/');
  QVariantMap ipv4address;
  ipv4address.insert("address", ipv4split.first());
  if (ipv4split.length() > 1) {
    ipv4address.insert("prefix", ipv4split[1].toUInt());
  } else {
    ipv4address.insert("prefix", (uint)32);
  }
  m_ipv4config.insert("method", "manual");
  m_ipv4config.insert("address-data", NetmgrDataList(ipv4address));

  QStringList ipv6split = device->ipv6Address().split('/');
  QVariantMap ipv6address;
  ipv6address.insert("address", ipv6split.first());
  if (ipv6split.length() > 1) {
    ipv6address.insert("prefix", ipv6split[1].toUInt());
  } else {
    ipv6address.insert("prefix", (uint)128);
  }
  m_ipv6config.insert("method", "manual");
  m_ipv6config.insert("address-data", NetmgrDataList(ipv6address));

  m_wireguard.insert("fwmark", WG_FIREWALL_MARK);
  m_wireguard.insert("ip4-auto-default-route", 1);
  m_wireguard.insert("ip6-auto-default-route", 1);
  m_wireguard.insert("peer-routes", false);
  m_wireguard.insert("private-key", keys->privateKey());

  QString path = QStringLiteral(DBUS_NM_PATH) + "/Settings";
  QDBusInterface iface(DBUS_NM_SERVICE, path, nmInterface("Settings"),
                       m_client->connection());
  if (!iface.isValid()) {
    logger.warning() << "NetworkManager settings interface is not available";
    emit initialized(false, false, QDateTime());
    return;
  }

  // Monitor for changes in the connected devices.
  connect(m_client, SIGNAL(DeviceAdded(QDBusObjectPath)), this,
          SLOT(deviceAdded(QDBusObjectPath)));
  connect(m_client, SIGNAL(DeviceRemoved(QDBusObjectPath)), this,
          SLOT(deviceRemoved(QDBusObjectPath)));

  // DeviceAdded only reports new devices, so pick up wg0 if it already exists.
  QDBusReply<QDBusObjectPath> devReply =
      m_client->call("GetDeviceByIpIface", WG_INTERFACE_NAME);
  if (devReply.isValid()) {
    m_device = new NetmgrDevice(devReply.value().path(), this);
    connect(m_device, &NetmgrDevice::stateChanged, this,
            &NetmgrController::deviceStateChanged);
  }

  // Delete any profile left over from a previous session, it should not exist
  // and can't be trusted. If obfuscation was in use the process died with that
  // session: the WireGuard peer points to a dead 127.0.0.1:<port> and we have
  // no InterfaceConfig to rebuild the obfuscator.
  QDBusReply<QDBusObjectPath> reply = iface.call("GetConnectionByUuid", m_uuid);
  if (reply.isValid()) {
    logger.info() << "Deleting stale connection:" << m_uuid;
    // activate() must not add the new connection until the delete finishes,
    // or AddConnection2 fails on the duplicate UUID.
    setProfileState(ProfileState::Cleaning);
    QDBusMessage del = QDBusMessage::createMethodCall(
        DBUS_NM_SERVICE, reply.value().path(),
        nmInterface("Settings.Connection"), "Delete");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(
        m_client->connection().asyncCall(del), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this,
             stalePath = reply.value().path()](QDBusPendingCallWatcher* call) {
              auto guard = qScopeGuard([call] { call->deleteLater(); });
              if (call->isError()) {
                // The profile still exists, so AddConnection2 would fail on
                // the duplicate UUID. Reuse it instead, Update2 overwrites its
                // settings before it is activated.
                logger.warning() << "Failed to delete stale connection:"
                                 << call->error().message();
                m_remote =
                    new QDBusInterface(DBUS_NM_SERVICE, stalePath,
                                       nmInterface("Settings.Connection"),
                                       m_client->connection(), this);
                setProfileState(ProfileState::Ready);
              } else {
                setProfileState(ProfileState::Absent);
              }
              advance();
            });
  }

  // Interface creation is deferred until the first activation.

  emit initialized(true, false, QDateTime());
}

void NetmgrController::setProfileState(ProfileState state) {
  logger.debug() << "Profile state:" << m_profileState << "->" << state;
  m_profileState = state;
}

// Take the next step towards the user's intent, given the profile state.
// Called whenever the intent or the profile state changes.
void NetmgrController::advance() {
  if (!m_wantActive) {
    return;
  }
  switch (m_profileState) {
    case ProfileState::Absent:
      addConnection();
      break;
    case ProfileState::Cleaning:
    case ProfileState::Adding:
      // Wait for the pending reply, it calls advance() again.
      break;
    case ProfileState::Ready:
      updateConnection();
      break;
  }
}

void NetmgrController::dbusBackendError(const QDBusError& error) {
  logger.warning() << "DBus message failed:" << error.message();
  if (!m_wantActive) {
    // The user already disconnected; nothing to fail.
    return;
  }
  emit backendFailure(Controller::ErrorFatal);
}

// static
QVariantMap NetmgrController::wgPeer(const InterfaceConfig& config) {
  QStringList ipList;
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    ipList.append(i.toString());
  }

  QVariantMap peer;
  QString endpoint;
  if (!config.m_serverIpv4AddrIn.isNull()) {
    endpoint =
        config.m_serverIpv4AddrIn + ":" + QString::number(config.m_serverPort);
  } else {
    endpoint = "[" + config.m_serverIpv6AddrIn +
               "]:" + QString::number(config.m_serverPort);
  }
  peer.insert("endpoint", endpoint);
  peer.insert("persistent-keepalive", WG_KEEPALIVE_PERIOD);
  peer.insert("public-key", config.m_serverPublicKey);
  peer.insert("allowed-ips", ipList);
  return peer;
}

// static
void NetmgrController::setDnsConfig(QVariantMap& map,
                                    const QHostAddress& server) {
  if (server.isNull()) {
    map.remove("dns");
    map.remove("dns-priority");
    map.remove("dns-search");
  } else if (server.protocol() == QAbstractSocket::IPv6Protocol) {
    NetmgrIpv6List dnsServers;
    dnsServers.append(server.toIPv6Address());
    map.insert("dns", dnsServers);
    map.insert("dns-priority", 10);
    map.insert("dns-search", QStringList("."));
  } else {
    QList<uint> dnsServers;
    dnsServers.append(qToBigEndian<quint32>(server.toIPv4Address()));
    map.insert("dns", QVariant::fromValue(dnsServers));
    map.insert("dns-priority", 10);
    map.insert("dns-search", QStringList("."));
  }
}

QVariant NetmgrController::serializeConfig() const {
  NetmgrConfig config;
  config.insert("connection", m_config);
  config.insert("ipv4", m_ipv4config);
  config.insert("ipv6", m_ipv6config);
  config.insert("wireguard", m_wireguard);
  return QVariant::fromValue(config);
}

void NetmgrController::activate(const InterfaceConfig& config,
                                Controller::Reason reason) {
  if (!m_client || !m_client->isValid()) {
    logger.warning() << "Cannot activate without a valid NetworkManager "
                        "connection";
    emit backendFailure(Controller::ErrorFatal);
    return;
  }
  const bool obfuscate =
      config.m_obfuscationMethod != Server::ObfuscationMethod::NoObfuscation &&
      config.m_hopType != InterfaceConfig::MultiHopExit;

  // Update routes and allowedIpAddreses
  NetmgrDataList ipv4routes;
  NetmgrDataList ipv6routes;
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    QVariantMap route;
    route.insert("dest", i.toHostString());
    route.insert("prefix", (uint)i.prefixLength());
    route.insert("table", WG_FIREWALL_MARK);
    if (i.protocol() == QAbstractSocket::IPv6Protocol) {
      ipv6routes.append(route);
    } else {
      ipv4routes.append(route);
    }
  }

  // The traffic to the real server must bypass the tunnel.
  // In the Flatpak sandbox, we cannot use SO_MARK so instead we set
  // "throw" route for each server endpoint into the WireGuard policy table
  // "throw" needs NetworkManager >= 1.38.
  if (obfuscate) {
    if (!obfuscationSupported()) {
      logger.warning() << "NetworkManager" << m_version.toString()
                       << "is too old for obfuscation bypass routes;"
                       << "server traffic may loop back into wg0";
      emit backendFailure(Controller::ErrorFatal);
      return;
    } else {
      for (const QString& addr :
           {config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn}) {
        if (addr.isEmpty()) {
          continue;
        }
        QVariantMap route;
        route.insert("dest", addr);
        route.insert("type", QStringLiteral("throw"));
        route.insert("table", WG_FIREWALL_MARK);
        if (QHostAddress(addr).protocol() == QAbstractSocket::IPv6Protocol) {
          route.insert("prefix", (uint)128);
          ipv6routes.append(route);
        } else {
          route.insert("prefix", (uint)32);
          ipv4routes.append(route);
        }
      }
    }
  }

  // Start the obfuscator proxy and point the WireGuard peer at it.
  InterfaceConfig peerConfig = config;
  m_obfuscator.reset();
  if (obfuscate) {
    auto obfuscator = std::make_unique<QProcessObfuscator>(config);
    if (!obfuscator->start()) {
      logger.error() << "Failed to start obfuscator"
                     << config.m_obfuscationMethod;
      emit backendFailure(Controller::ErrorFatal);
      return;
    }
    peerConfig.m_serverIpv4AddrIn = "127.0.0.1";
    // The relay only binds 127.0.0.1, so clear the IPv6 endpoint to stop
    // WireGuard from selecting [::1].
    peerConfig.m_serverIpv6AddrIn = QString();
    peerConfig.m_serverPort = obfuscator->localPort();
    m_obfuscator = std::move(obfuscator);
  }

  NetmgrDataList peers(wgPeer(peerConfig));
  m_ipv4config.insert("route-data", ipv4routes);
  m_ipv6config.insert("route-data", ipv6routes);
  m_wireguard.insert("peers", peers);

  // Keep the server details for later.
  m_serverPublicKey = config.m_serverPublicKey;
  m_serverIpv4Gateway = QHostAddress(config.m_serverIpv4Gateway);
  m_serverIpv6Gateway = QHostAddress(config.m_serverIpv6Gateway);

  // Update the DNS server.
  if ((config.m_dnsServer == config.m_serverIpv4Gateway) ||
      (config.m_dnsServer == config.m_serverIpv6Gateway)) {
    setDnsConfig(m_ipv4config, m_serverIpv4Gateway);
    setDnsConfig(m_ipv6config, m_serverIpv6Gateway);
  } else if (config.m_dnsServer.contains(':')) {
    setDnsConfig(m_ipv4config, QHostAddress());
    setDnsConfig(m_ipv6config, QHostAddress(config.m_dnsServer));
  } else {
    setDnsConfig(m_ipv4config, QHostAddress(config.m_dnsServer));
    setDnsConfig(m_ipv6config, QHostAddress());
  }

  m_wantActive = true;
  advance();
}

void NetmgrController::addConnection() {
  logger.info() << "Adding connection:" << m_uuid;
  Q_ASSERT(m_profileState == ProfileState::Absent);
  setProfileState(ProfileState::Adding);
  QString path = QStringLiteral(DBUS_NM_PATH) + "/Settings";
  QDBusInterface iface(DBUS_NM_SERVICE, path, nmInterface("Settings"),
                       m_client->connection());
  QVariantList addArgs;
  addArgs << serializeConfig();
  addArgs << (uint)IN_MEMORY;
  addArgs << QVariantMap();
  QDBusPendingReply<QDBusObjectPath, QVariantMap> added =
      iface.asyncCallWithArgumentList("AddConnection2", addArgs);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(added, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher* call) {
            auto guard = qScopeGuard([call] { call->deleteLater(); });
            QDBusPendingReply<QDBusObjectPath, QVariantMap> added = *call;
            if (!added.isValid()) {
              logger.warning()
                  << "AddConnection2 failed:" << added.error().message();
              // Don't advance() here, it would retry forever. The next
              // activate() tries again from Absent.
              setProfileState(ProfileState::Absent);
              if (m_wantActive) {
                emit backendFailure(Controller::ErrorFatal);
              }
              return;
            }
            // Keep the connection even if the user disconnected meanwhile,
            // the next activate() reuses it.
            m_remote = new QDBusInterface(DBUS_NM_SERVICE,
                                          added.argumentAt<0>().path(),
                                          nmInterface("Settings.Connection"),
                                          m_client->connection(), this);
            setProfileState(ProfileState::Ready);
            advance();
          });
}

void NetmgrController::updateConnection() {
  Q_ASSERT(m_remote);

  QList<QVariant> args;
  args << serializeConfig();
  args << (uint)IN_MEMORY;
  args << QVariantMap();
  QDBusPendingReply<QVariantMap> reply =
      m_remote->asyncCallWithArgumentList("Update2", args);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher* call) {
            auto guard = qScopeGuard([call] { call->deleteLater(); });
            QDBusPendingReply<QVariantMap> reply = *call;
            if (!reply.isValid()) {
              dbusBackendError(reply.error());
              return;
            }
            peerCompleted(reply.value());
          });
}

void NetmgrController::peerCompleted(const QVariantMap& results) {
  logger.debug() << "Peer configured";
  for (auto i = results.constBegin(); i != results.constEnd(); i++) {
    logger.debug() << "peer result:" << i.key() << "->" << i.value().toString();
  }

  if (!m_wantActive) {
    logger.info() << "Deactivated while updating the connection";
    return;
  }

  // activate the vpn
  if (!m_client || !m_client->isValid() || !m_remote || !m_remote->isValid()) {
    logger.warning() << "Cannot activate without a valid NetworkManager "
                        "connection";
    emit backendFailure(Controller::ErrorFatal);
    return;
  }

  QList<QVariant> args;
  args << QDBusObjectPath(m_remote->path());
  args << QDBusObjectPath("/");
  args << QDBusObjectPath("/");
  QDBusPendingReply<QDBusObjectPath> reply =
      m_client->asyncCallWithArgumentList("ActivateConnection", args);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher* call) {
            auto guard = qScopeGuard([call] { call->deleteLater(); });
            QDBusPendingReply<QDBusObjectPath> reply = *call;
            if (!reply.isValid()) {
              dbusBackendError(reply.error());
              return;
            }
            activateCompleted(reply.value());
          });
}

void NetmgrController::activateCompleted(const QDBusObjectPath& path) {
  logger.info() << "Connection activated:" << path.path();

  if (!m_wantActive) {
    // deactivate() ran while ActivateConnection was in progress. It found no
    // device and reported disconnected already, so take the tunnel down here.
    logger.info() << "Deactivated while activating; tearing down";
    QDBusMessage call = QDBusMessage::createMethodCall(
        DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE,
        "DeactivateConnection");
    call << path;
    m_client->connection().asyncCall(call);
  }
}

void NetmgrController::deactivate() {
  m_wantActive = false;

  // Stop the obfuscator relay but keep the instance, so it can be restarted on
  // the same local port if the tunnel is reactivated from NetworkManager.
  if (m_obfuscator) {
    m_obfuscator->stop();
  }

  if (!m_device || m_device->uuid() != m_uuid) {
    logger.warning() << "Client already disconnected";
    emit disconnected();
    return;
  }

  QDBusMessage call = QDBusMessage::createMethodCall(
      DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE, "DeactivateConnection");
  call << QDBusObjectPath(m_device->activeConnection());

  QDBusPendingReply<> reply = m_client->connection().asyncCall(call);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                   [&] { emit disconnected(); });
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher,
                   &QObject::deleteLater);
}

void NetmgrController::deviceAdded(const QDBusObjectPath& devpath) {
  NetmgrDevice* device = new NetmgrDevice(devpath.path(), this);
  auto guard = qScopeGuard([device]() { delete device; });

  logger.debug() << "device added:" << devpath.path();
  logger.debug() << "interface:" << device->name() << device->driver();
  if (device->name() != WG_INTERFACE_NAME) {
    return;
  }
  if (device->driver() != "wireguard") {
    return;
  }

  // Update the current device.
  if (m_device) {
    delete m_device;
  }
  m_device = device;
  guard.dismiss();

  // Watch it for state changes.
  connect(m_device, &NetmgrDevice::stateChanged, this,
          &NetmgrController::deviceStateChanged);
  deviceStateChanged(m_device->state(), NetmgrDevice::UNKNOWN, 0);
}

void NetmgrController::deviceRemoved(const QDBusObjectPath& path) {
  logger.debug() << "device removed:" << path.path();
  if (m_device && m_device->path() == path.path()) {
    delete m_device;
    m_device = nullptr;

    if (m_obfuscator) {
      m_obfuscator->stop();
    }

    m_connectionPath = "/";
    emit disconnected();
  }
}

void NetmgrController::deviceStateChanged(uint state, uint prev, uint reason) {
  auto newstate = static_cast<NetmgrDevice::State>(state);
  auto prevstate = static_cast<NetmgrDevice::State>(prev);
  logger.debug() << "device state changed:" << prevstate << "->" << newstate;

  // Stop the relay when the tunnel is deactivated from NetworkManager, which
  // bypasses deactivate(). This runs before the uuid check because the device
  // drops its active connection, and with it the uuid, as it disconnects.
  // A new activation replacing this one keeps the relay it was set up with.
  if (m_obfuscator && prevstate >= NetmgrDevice::PREPARE &&
      prevstate <= NetmgrDevice::ACTIVATED &&
      (newstate < NetmgrDevice::PREPARE ||
       newstate > NetmgrDevice::ACTIVATED) &&
      reason != NM_DEVICE_STATE_REASON_NEW_ACTIVATION) {
    logger.info() << "Tunnel deactivated; stopping obfuscator";
    m_obfuscator->stop();
  }

  if (m_device->uuid() != m_uuid) {
    return;
  }
  if (m_device->activeConnection() == m_relayFailedConnection) {
    return;
  }

  // The connection can be toggled directly from NetworkManager, bypassing
  // activate()/deactivate(). When obfuscation is in use, the relay
  // process may have exited while the tunnel was down, but the connection
  // still points at 127.0.0.1:<port>.
  // Bring the relay back on the same local port while the tunnel activates.
  // Check every activation state, not only the transition into PREPARE: the
  // device's uuid comes from a separate PropertiesChanged signal and may only
  // match from a later transition on.
  if (m_obfuscator && newstate >= NetmgrDevice::PREPARE &&
      newstate <= NetmgrDevice::ACTIVATED && !m_obfuscator->isRunning()) {
    logger.warning() << "Obfuscator not running on activation; restarting on"
                     << "port" << m_obfuscator->localPort();
    if (!m_obfuscator->restart()) {
      // Without the relay the tunnel carries no traffic. Take it down so the
      // later transition to ACTIVATED doesn't report connected().
      logger.error() << "Failed to restart obfuscator";
      m_relayFailedConnection = m_device->activeConnection();
      QDBusMessage call = QDBusMessage::createMethodCall(
          DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE,
          "DeactivateConnection");
      call << QDBusObjectPath(m_relayFailedConnection);
      m_client->connection().asyncCall(call);
      emit backendFailure(Controller::ErrorFatal);
      return;
    }
  }

  if (newstate == NetmgrDevice::ACTIVATED) {
    emit connected(m_serverPublicKey);
  }
}

// static
uint64_t NetmgrController::readSysfsFile(const QString& path) {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0;
  }
  QString line = QString::fromLocal8Bit(file.readLine());
  file.close();

  return line.toULongLong();
}

void NetmgrController::checkStatus() {
  QString txPath =
      QString("/sys/class/net/%1/statistics/tx_bytes").arg(WG_INTERFACE_NAME);
  QString rxPath =
      QString("/sys/class/net/%1/statistics/rx_bytes").arg(WG_INTERFACE_NAME);

  ControllerStatus st;
  st.m_connected = true;
  st.m_timestamp = guessUptime();
  st.m_ipv4Gateway = m_serverIpv4Gateway;
  st.m_ipv6Gateway = m_serverIpv6Gateway;
  st.m_ipv4Address = m_deviceIpv4Address;
  st.m_ipv6Address = m_deviceIpv6Address;
  st.m_rxBytes = readSysfsFile(rxPath);
  st.m_txBytes = readSysfsFile(txPath);

  emit statusUpdated(st);
}

QDateTime NetmgrController::guessUptime() {
  QString devPath = QString("/sys/class/net/%1").arg(WG_INTERFACE_NAME);
  struct stat st;
  if (lstat(qPrintable(devPath), &st) != 0) {
    return QDateTime();
  } else {
    logger.debug() << "got st_mtime:" << st.st_mtime;
    return QDateTime::fromSecsSinceEpoch(st.st_mtime);
  }
}
