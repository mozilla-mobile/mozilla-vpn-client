/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanagercontroller.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QFile>
#include <QScopeGuard>
#include <QUuid>

#include "dbustypes.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "netmgrconnection.h"
#include "netmgrtypes.h"
#include "settingsholder.h"

constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
constexpr const char* WG_INTERFACE_NAME = "wg0";
constexpr uint16_t WG_KEEPALIVE_PERIOD = 60;
constexpr uint16_t WG_EXCLUDE_RULE_PRIO = 100;

const QString DBUS_NM_SERVICE = QStringLiteral("org.freedesktop.NetworkManager");
const QString DBUS_NM_PATH = QStringLiteral("/org/freedesktop/NetworkManager");
const QString DBUS_NM_INTERFACE = QStringLiteral("org.freedesktop.NetworkManager");
const QString DBUS_PROPERTY_INTERFACE = QStringLiteral("org.freedesktop.DBus.Properties");

namespace {
Logger logger("NetworkManagerController");
}

NetworkManagerController::NetworkManagerController() {
  MZ_COUNT_CTOR(NetworkManagerController);

  m_client = new QDBusInterface(DBUS_NM_SERVICE, DBUS_NM_PATH,
                                DBUS_NM_INTERFACE, QDBusConnection::systemBus(),
                                this);
  m_settings = new QDBusInterface(DBUS_NM_SERVICE, DBUS_NM_PATH + "/Settings",
                                  DBUS_NM_INTERFACE + ".Settings",
                                  QDBusConnection::systemBus(), this);
  
  QVariant version = m_client->property("Version");
  m_version = QVersionNumber::fromString(version.toString());
  logger.info() << "NetworkManager version:" << m_version.toString();

  // Watch for property changes
  QDBusConnection::systemBus().connect(
      DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_PROPERTY_INTERFACE,
      "PropertiesChanged", this,
      SLOT(propertyChanged(QString, QVariantMap, QStringList)));
}

NetworkManagerController::~NetworkManagerController() {
  MZ_COUNT_DTOR(NetworkManagerController);
  logger.debug() << "Destroying NetworkManagerController";

  // Clear the active connection, if any.
  setActiveConnection(QString());
}

void NetworkManagerController::initialize(const Device* device,
                                          const Keys* keys) {
  // Ensure we use a consistent UUID for the wireguard interface.
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  QUuid uuid(settingsHolder->tunnelUuid());
  if (uuid.isNull()) {
    uuid = QUuid::createUuid();
    settingsHolder->setTunnelUuid(uuid.toString(QUuid::WithoutBraces));
  }
  m_tunnelUuid = uuid.toString(QUuid::WithoutBraces);

  m_deviceIpv4Address = device->ipv4Address();

  // Generic connection settings.
  m_config.insert("id", QCoreApplication::applicationName());
  m_config.insert("interface-name", QString(WG_INTERFACE_NAME));
  m_config.insert("type", QString("wireguard"));
  m_config.insert("uuid", m_tunnelUuid);
  m_config.insert("autoconnect", false);
  // TODO: Permissions?
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
  m_ipv4config.insert("address-data", NetMgrDataList(ipv4address).toVariant());

  QStringList ipv6split = device->ipv6Address().split('/');
  QVariantMap ipv6address;
  ipv6address.insert("address", ipv6split.first());
  if (ipv6split.length() > 1) {
    ipv6address.insert("prefix", ipv6split[1].toUInt());
  } else {
    ipv6address.insert("prefix", (uint)128);
  }
  m_ipv6config.insert("method", "manual");
  m_ipv6config.insert("address-data", NetMgrDataList(ipv6address).toVariant());

  m_wireguard.insert("fwmark", WG_FIREWALL_MARK);
  m_wireguard.insert("ip4-auto-default-route", 1);
  m_wireguard.insert("ip6-auto-default-route", 1);
  m_wireguard.insert("peer-routes", false);
  m_wireguard.insert("private-key", keys->privateKey());

  // Check if the connection already exists.
  QDBusReply<QDBusObjectPath> reply = m_settings->call("GetConnectionByUuid",
                                                       m_tunnelUuid);
  if (reply.isValid()) {
    logger.info() << "Connection" << m_tunnelUuid << "already exists";
    QString path = reply.value().path();
    m_remote = new QDBusInterface(DBUS_NM_SERVICE, reply.value().path(),
                                  DBUS_NM_INTERFACE + ".Settings.Connection",
                                  QDBusConnection::systemBus(), this);
#if 0
    // Lookup the active connection handle, or null if the connection is down.
    const GPtrArray* connections = nm_client_get_active_connections(m_libnmclient);
    for (guint i = 0; i < connections->len; i++) {
      NMActiveConnection* active = NM_ACTIVE_CONNECTION(connections->pdata[i]);
      if (m_tunnelUuid == nm_active_connection_get_uuid(active)) {
        setActiveConnection(nm_object_get_path(NM_OBJECT(active)));
        break;
      }
    }
#endif

    emit initialized(true, m_connection != nullptr, QDateTime());
    return;
  } else {
    // Create the connection
    logger.info() << "Creating connection:" << m_tunnelUuid;
    QVariantList args;
    args << serializeConfig();
    args << (uint)0x02;
    args << QVariantMap();
    bool okay = m_settings->callWithCallback(
        "AddConnection2", args, this,
        SLOT(initCompleted(const QDBusObjectPath&, const QVariantMap&)),
        SLOT(dbusError(const QDBusError&)));
    if (!okay) {
      logger.debug() << "AddConnection2 failed";
    }
  }
}

void NetworkManagerController::dbusError(const QDBusError& error) {
  logger.warning() << "D-Bus message failed:" << error.message();
  emit backendFailure(Controller::ErrorFatal);
}

// static
QVariantMap NetworkManagerController::wgPeer(const InterfaceConfig& config) {
  QStringList ipList;
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    ipList.append(i.toString());
  }

  QVariantMap peer;
  QString endpoint = config.m_serverIpv4AddrIn + ":" + QString::number(config.m_serverPort);
  peer.insert("endpoint", endpoint);
  peer.insert("persistent-keepalive", WG_KEEPALIVE_PERIOD);
  peer.insert("public-key", config.m_serverPublicKey);
  peer.insert("allowed-ips", ipList);
  return peer;
}

// static
void NetworkManagerController::setDnsConfig(QVariantMap& map, const QHostAddress& server) {
  if (server.isNull()) {
    map.remove("dns");
    map.remove("dns-priority");
    map.remove("dns-search");
  } else if (server.protocol() == QAbstractSocket::IPv6Protocol) {
    NetMgrIpv6List dnsServers;
    dnsServers.append(server.toIPv6Address());
    map.insert("dns", dnsServers.toVariant());
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

QVariant NetworkManagerController::serializeConfig() const {
  NetMgrConfig config;
  config.insert("connection", m_config);
  config.insert("ipv4", m_ipv4config);
  config.insert("ipv6", m_ipv6config);
  config.insert("wireguard", m_wireguard);
  return QVariant::fromValue(config);
}

void NetworkManagerController::initCompleted(const QDBusObjectPath& path,
                                             const QVariantMap& results) {
  m_remote = new QDBusInterface(DBUS_NM_SERVICE, path.path(),
                                DBUS_NM_INTERFACE + ".Service.Connection",
                                QDBusConnection::systemBus(), this);
  emit initialized(m_remote != nullptr, false, QDateTime());
}

void NetworkManagerController::activate(const InterfaceConfig& config,
                                        Controller::Reason reason) {
  // Update routes and allowedIpAddreses
  NetMgrDataList ipv4routes;
  NetMgrDataList ipv6routes;
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    QVariantMap route;
    route.insert("dest", i.address().toString());
    route.insert("prefix", (uint)i.prefixLength());
    route.insert("table", WG_FIREWALL_MARK);
    if (i.address().protocol() == QAbstractSocket::IPv6Protocol) {
      ipv6routes.append(route);
    } else {
      ipv4routes.append(route);
    }
  }

  NetMgrDataList peers(wgPeer(config));
  m_ipv4config.insert("route-data", ipv4routes.toVariant());
  m_ipv6config.insert("route-data", ipv6routes.toVariant());
  m_wireguard.insert("peers", peers.toVariant());

  // Update the DNS server.
  if ((config.m_dnsServer == config.m_serverIpv4Gateway) ||
      (config.m_dnsServer == config.m_serverIpv6Gateway)) {
    setDnsConfig(m_ipv4config, QHostAddress(config.m_serverIpv4Gateway));
    setDnsConfig(m_ipv6config, QHostAddress(config.m_serverIpv6Gateway));
  } else if (config.m_dnsServer.contains(':')) {
    setDnsConfig(m_ipv4config, QHostAddress());
    setDnsConfig(m_ipv6config, QHostAddress(config.m_dnsServer));
  } else {
    setDnsConfig(m_ipv4config, QHostAddress(config.m_dnsServer));
    setDnsConfig(m_ipv6config, QHostAddress());
  }

  // Keep the server details for later.
  m_serverPublicKey = config.m_serverPublicKey;
  m_serverIpv4Gateway = config.m_serverIpv4Gateway;

  // Update the connection settings.
  QList<QVariant> args;
  args << serializeConfig();
  args << (uint)0x02;
  args << QVariantMap();
  bool okay = m_remote->callWithCallback("Update2", args, this,
                                         SLOT(peerCompleted(const QVariantMap&)),
                                         SLOT(dbusError(const QDBusError&)));
  if (!okay) {
    logger.debug() << "Update2 failed";
  }
}

void NetworkManagerController::peerCompleted(const QVariantMap& result) {
  // activate the vpn
  QList<QVariant> args;
  args << QDBusObjectPath(m_remote->path());
  args << QDBusObjectPath("/");
  args << QDBusObjectPath("/");
  bool okay = m_client->callWithCallback("ActivateConnection", args, this,
                                         SLOT(activateCompleted(const QDBusObjectPath&)),
                                         SLOT(dbusError(const QDBusError&)));
  if (!okay) {
    logger.debug() << "ActivateConnection failed";
  }
}

void NetworkManagerController::activateCompleted(const QDBusObjectPath& path) {
  setActiveConnection(path.path());
}

void NetworkManagerController::setActiveConnection(const QString& path) {
  // If there is an existing active connection, discard it.
  if (m_connection != nullptr) {
    if (m_connection->path() == path) {
      // No change - do nothing.
      return;
    }
    delete m_connection;
    m_connection = nullptr;
  }

  // Start monitoring the new connection for state changes.
  if (!path.isEmpty()) {
    m_connection = new NetMgrConnection(path, this);
    connect(m_connection, &NetMgrConnection::stateChanged, this,
            &NetworkManagerController::stateChanged);

    // Invoke the state changed signal with the current state.
    stateChanged(m_connection->state(), 0);
  }
}

void NetworkManagerController::stateChanged(uint state, uint reason) {
  auto nmstate = (NetMgrConnection::ActiveState)state;
  logger.debug() << "DBus state changed" << nmstate << reason;
  switch (nmstate) {
    case NetMgrConnection::NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
      emit connected(m_serverPublicKey);
      break;
      
    case NetMgrConnection::NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
      emit disconnected();
      break;
    
    default:
      break;
  }
}

void NetworkManagerController::deactivate(Controller::Reason reason) {
  Q_UNUSED(reason);

  if (m_connection == nullptr) {
    logger.warning() << "Client already disconnected";
    emit disconnected();
    return;
  }

  QDBusMessage call = QDBusMessage::createMethodCall(
      DBUS_NM_SERVICE, DBUS_NM_PATH, DBUS_NM_INTERFACE, "DeactivateConnection");
  call << QDBusObjectPath(m_connection->path());

  QDBusPendingReply<> reply = QDBusConnection::systemBus().asyncCall(call);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                   [&] { emit disconnected(); });
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher,
                   &QObject::deleteLater);

  delete m_connection;
  m_connection = nullptr;
}

void NetworkManagerController::propertyChanged(QString interface,
                                               QVariantMap props,
                                               QStringList list) {
  Q_UNUSED(list);
  if (interface != DBUS_NM_INTERFACE) {
    return;
  }

  // If the ActivatingConnection changed, check to see if our interface was
  // activated externally.
  if (props.contains("ActivatingConnection")) {
    QDBusObjectPath path =
        props.value("ActivatingConnection").value<QDBusObjectPath>();

    // Is this the tunnel inteface?
    QDBusInterface conn(DBUS_NM_SERVICE, path.path(), DBUS_NM_INTERFACE + ".Active",
                        QDBusConnection::systemBus());
    QString uuid = conn.property("Uuid").toString();
    logger.debug() << "Connection" << uuid << "started";
    if (m_tunnelUuid == uuid) {
      setActiveConnection(path.path());
    }
  }
}

// static
uint64_t NetworkManagerController::readSysfsFile(const QString& path) {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return 0;
  }
  QString line = QString::fromLocal8Bit(file.readLine());
  file.close();

  return line.toULongLong();
}

void NetworkManagerController::checkStatus() {
  QString txPath =
      QString("/sys/class/net/%1/statistics/tx_bytes").arg(WG_INTERFACE_NAME);
  QString rxPath =
      QString("/sys/class/net/%1/statistics/rx_bytes").arg(WG_INTERFACE_NAME);
  uint64_t txBytes = readSysfsFile(txPath);
  uint64_t rxBytes = readSysfsFile(rxPath);
  logger.info() << "Status:" << m_deviceIpv4Address << txBytes << rxBytes;
  emit statusUpdated(m_serverIpv4Gateway, m_deviceIpv4Address, txBytes, rxBytes);
}
