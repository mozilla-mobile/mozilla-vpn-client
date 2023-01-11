/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxnmcontroller.h"

#include <QScopeGuard>
#include <QUuid>

#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "settingsholder.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include <glib.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#undef signals
#include <NetworkManager.h>
#define signals Q_SIGNALS

#if defined(__cplusplus)
};
#endif

constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
constexpr const char* WG_INTERFACE_NAME = "wg0";
constexpr uint16_t WG_KEEPALIVE_PERIOD = 60;
constexpr uint16_t WG_EXCLUDE_RULE_PRIO = 100;

namespace {
Logger logger("LinuxNMController");
}

LinuxNMController::LinuxNMController() {
  MZ_COUNT_CTOR(LinuxNMController);

  GError* err = nullptr;
  m_client = nm_client_new(nullptr, &err);
  if (m_client) {
    logger.info() << "NetworkManager version:"
                  << nm_client_get_version(m_client);
  } else {
    logger.error() << "Failed to create NetworkManager client:" << err->message;
    g_error_free(err);
  }

  m_wireguard = nm_setting_wireguard_new();
  m_cancellable = g_cancellable_new();
}

LinuxNMController::~LinuxNMController() {
  MZ_COUNT_DTOR(LinuxNMController);
  logger.debug() << "Destroying LinuxNMController";
  g_cancellable_cancel(m_cancellable);

  if (m_remote) {
    nm_remote_connection_delete_async(
        m_remote, nullptr,
        [](GObject* obj, GAsyncResult* res, gpointer data) {
          Q_UNUSED(obj);
          GError* err = nullptr;
          if (!nm_remote_connection_delete_finish(NM_REMOTE_CONNECTION(data),
                                                  res, &err)) {
            logger.error() << "Connection deletion failed:" << err->message;
            g_error_free(err);
          }
        },
        m_remote);
  }

  if (m_ipv6config != nullptr) {
    g_object_unref(m_ipv6config);
  }
  if (m_ipv4config != nullptr) {
    g_object_unref(m_ipv4config);
  }
  g_object_unref(m_cancellable);
  g_object_unref(m_wireguard);
  g_object_unref(m_client);
}

// GAsyncCallback helper function, takes a reference to the async result
// and queues it for consumption by the controller class.
static void asyncResultWrapper(GAsyncResult* result, const char* name) {
  LinuxNMController* controller =
      static_cast<LinuxNMController*>(g_async_result_get_user_data(result));
  GAsyncResult* reference = g_object_ref(result);
  bool invoked = QMetaObject::invokeMethod(
      controller, name, Qt::QueuedConnection, Q_ARG(void*, reference));
  if (!invoked) {
    logger.error() << "Failed to invoke" << name;
    g_object_unref(reference);
  }
}

// A macro to ensure the function signature matches a GAsyncCallback
#define LAMBDA_ASYNC_WRAPPER(__name__)                    \
  [](GObject* obj, GAsyncResult* result, gpointer data) { \
    asyncResultWrapper(result, __name__);                 \
  }

// Helper function to parse an IP prefix into an NMIPAddress
static NMIPAddress* parseAddress(const QString& addr) {
  uint prefix = -1;
  int slash = addr.indexOf('/');
  QString base = addr.first(slash);

  if (slash > 0) {
    prefix = addr.mid(slash + 1).toUInt();
  }

  if (base.contains(':')) {
    if (prefix < 0) prefix = 128;
    return nm_ip_address_new(AF_INET6, qPrintable(base), prefix, nullptr);
  } else {
    if (prefix < 0) prefix = 32;
    return nm_ip_address_new(AF_INET, qPrintable(base), prefix, nullptr);
  }
}

// Lookup the active connection handle, or null if the connection is down.
NMActiveConnection* LinuxNMController::getActiveConnection() const {
  if (m_client == nullptr) {
    return nullptr;
  }

  const GPtrArray* active = nm_client_get_active_connections(m_client);
  for (int i = 0; i < active->len; i++) {
    NMActiveConnection* conn = NM_ACTIVE_CONNECTION(active->pdata[i]);
    if (m_tunnelUuid == nm_active_connection_get_uuid(conn)) {
      return conn;
    }
  }

  return nullptr;
}

void LinuxNMController::initialize(const Device* device, const Keys* keys) {
  NMConnection* wg_connection = nm_simple_connection_new();

  // Ensure we use a consistent UUID for the wireguard interface.
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  QUuid uuid(settingsHolder->tunnelUuid());
  if (uuid.isNull()) {
    uuid = QUuid::createUuid();
    settingsHolder->setTunnelUuid(uuid.toString(QUuid::WithoutBraces));
  }
  m_tunnelUuid = uuid.toString(QUuid::WithoutBraces);

  // Whether the connection exists or not, we will always overwrite
  // the wireguard settings with our own.
  g_object_set(m_wireguard, NM_SETTING_WIREGUARD_PRIVATE_KEY,
               qPrintable(keys->privateKey()), NM_SETTING_WIREGUARD_FWMARK,
               WG_FIREWALL_MARK, NM_SETTING_WIREGUARD_PEER_ROUTES, false, NULL);

  // Check if the connection already exists.
  NMRemoteConnection* remote =
      nm_client_get_connection_by_uuid(m_client, qPrintable(m_tunnelUuid));
  if (remote) {
    logger.info() << "Connection" << m_tunnelUuid << "already exists";
    m_remote = remote;

    NMSettingIPConfig* ipv4config =
        nm_connection_get_setting_ip4_config(NM_CONNECTION(remote));
    m_ipv4config = nm_setting_duplicate(NM_SETTING(ipv4config));

    NMSettingIPConfig* ipv6config =
        nm_connection_get_setting_ip6_config(NM_CONNECTION(remote));
    m_ipv6config = nm_setting_duplicate(NM_SETTING(ipv6config));

    emit initialized(true, getActiveConnection() != nullptr, QDateTime());
    return;
  }

  m_ipv4config = nm_setting_ip4_config_new();
  m_ipv6config = nm_setting_ip6_config_new();

  // Generic connection settings.
  NMSetting* scon = nm_setting_connection_new();
  nm_connection_add_setting(wg_connection, scon);
  g_object_set(scon, NM_SETTING_CONNECTION_ID, WG_INTERFACE_NAME,
               NM_SETTING_CONNECTION_TYPE, NM_SETTING_WIREGUARD_SETTING_NAME,
               NM_SETTING_CONNECTION_UUID, qPrintable(m_tunnelUuid),
               NM_SETTING_CONNECTION_INTERFACE_NAME, WG_INTERFACE_NAME,
               NM_SETTING_CONNECTION_AUTOCONNECT, false, NULL);

  // Address settings
  g_object_ref(m_ipv4config);
  g_object_set(m_ipv4config, NM_SETTING_IP_CONFIG_METHOD,
               NM_SETTING_IP4_CONFIG_METHOD_MANUAL, NULL);
  nm_connection_add_setting(wg_connection, m_ipv4config);
  nm_setting_ip_config_add_address(NM_SETTING_IP_CONFIG(m_ipv4config),
                                   parseAddress(device->ipv4Address()));

  g_object_ref(m_ipv6config);
  g_object_set(m_ipv6config, NM_SETTING_IP_CONFIG_METHOD,
               NM_SETTING_IP6_CONFIG_METHOD_MANUAL, NULL);
  nm_connection_add_setting(wg_connection, m_ipv6config);
  nm_setting_ip_config_add_address(NM_SETTING_IP_CONFIG(m_ipv6config),
                                   parseAddress(device->ipv6Address()));

  // Wireguard connection settings.
  g_object_ref(m_wireguard);
  nm_connection_add_setting(wg_connection, m_wireguard);

  // Create the connection
  logger.info() << "Creating connection:" << m_tunnelUuid;
  nm_client_add_connection2(
      m_client,
      nm_connection_to_dbus(wg_connection, NM_CONNECTION_SERIALIZE_ALL),
      NM_SETTINGS_ADD_CONNECTION2_FLAG_IN_MEMORY, nullptr, false, m_cancellable,
      LAMBDA_ASYNC_WRAPPER("initializeCompleted"), this);
}

void LinuxNMController::initializeCompleted(void* result) {
  GError* err = nullptr;
  GVariant* gv;

  m_remote = nm_client_add_connection2_finish(m_client, G_ASYNC_RESULT(result),
                                              &gv, &err);
  if (!m_remote) {
    logger.error() << "connection creation failed:" << err->message;
    g_error_free(err);
  } else {
    logger.debug() << "connection created:"
                   << nm_connection_get_interface_name(NM_CONNECTION(m_remote));
  }
  g_object_unref(result);

  emit initialized(m_remote != nullptr, false, QDateTime());
}

void LinuxNMController::activate(const HopConnection& hop, const Device* device,
                                 const Keys* keys, Controller::Reason reason) {
  NMWireGuardPeer* peer = nm_wireguard_peer_new();

  // Store the server's public key for later.
  m_serverPublicKey = hop.m_server.publicKey();

  // TODO: Support IPv6 endpoints too, or hop.m_server.hostname()?
  uint32_t serverPort = hop.m_server.choosePort();
  QString endpoint = QString("%1:%2").arg(hop.m_server.ipv4AddrIn(),
                                          QString::number(serverPort));

  nm_wireguard_peer_set_endpoint(peer, qPrintable(endpoint), true);
  nm_wireguard_peer_set_persistent_keepalive(peer, WG_KEEPALIVE_PERIOD);
  nm_wireguard_peer_set_public_key(peer, qPrintable(m_serverPublicKey), true);
  for (const IPAddress& i : hop.m_allowedIPAddressRanges) {
    int family;
    NMSettingIPConfig* ipcfg;
    if (i.address().protocol() == QAbstractSocket::IPv6Protocol) {
      family = AF_INET6;
      ipcfg = NM_SETTING_IP_CONFIG(m_ipv6config);
    } else {
      family = AF_INET;
      ipcfg = NM_SETTING_IP_CONFIG(m_ipv4config);
    }

    // Add routes manually so we can place them in the wireguard table.
    NMIPRoute* route;
    GError* err = nullptr;
    route = nm_ip_route_new(family, qPrintable(i.address().toString()),
                            i.prefixLength(), nullptr, -1, &err);
    if (route == nullptr) {
      logger.error() << "Failed to create route:" << err->message;
      g_error_free(err);
      continue;
    }
    nm_ip_route_set_attribute(route, NM_IP_ROUTE_ATTRIBUTE_TABLE,
                              g_variant_new_uint32(WG_FIREWALL_MARK));
    nm_setting_ip_config_add_route(ipcfg, route);
    nm_ip_route_unref(route);

    // Add the allowed IP to the wireguard peer.
    nm_wireguard_peer_append_allowed_ip(peer, qPrintable(i.toString()), false);
  }

  // Update the wireguard peer configuration.
  GError* err = nullptr;
  if (!nm_wireguard_peer_is_valid(peer, true, true, &err)) {
    logger.error() << "Invalid peer configuration:" << err->message;
    g_error_free(err);
  }
  nm_setting_wireguard_set_peer(NM_SETTING_WIREGUARD(m_wireguard), peer,
                                hop.m_hopindex);
  g_object_ref(m_wireguard);
  nm_connection_add_setting(NM_CONNECTION(m_remote), m_wireguard);

  if (hop.m_hopindex == 0) {
    // Update DNS when configuring the exit server.
    NMSettingIPConfig* ipcfg;
    if (hop.m_dnsServer.protocol() == QAbstractSocket::IPv6Protocol) {
      ipcfg = NM_SETTING_IP_CONFIG(m_ipv6config);
    } else {
      ipcfg = NM_SETTING_IP_CONFIG(m_ipv4config);
    }

    const char* dnsServerList[] = {qPrintable(hop.m_dnsServer.toString()),
                                   nullptr};
    const char* dnsSearchList[] = {".", nullptr};
    g_object_set(ipcfg, NM_SETTING_IP_CONFIG_DNS, dnsServerList,
                 NM_SETTING_IP_CONFIG_DNS_SEARCH, dnsSearchList,
                 NM_SETTING_IP_CONFIG_DNS_PRIORITY, 10, NULL);

    // Keep the IPv4 gateway for later.
    m_serverIpv4Gateway = hop.m_server.ipv4Gateway();
  }

  // Update excluded addresses.
  for (const QString& dst : hop.m_excludedAddresses) {
    NMIPRoute* route;
    NMSettingIPConfig* ipcfg;
    GError* err = nullptr;

    if (dst.contains(':')) {
      // Probably IPv6
      route =
          nm_ip_route_new(AF_INET6, qPrintable(dst), 128, nullptr, -1, &err);
      ipcfg = NM_SETTING_IP_CONFIG(m_ipv6config);
    } else {
      // Probably IPv4
      route = nm_ip_route_new(AF_INET, qPrintable(dst), 32, nullptr, -1, &err);
      ipcfg = NM_SETTING_IP_CONFIG(m_ipv4config);
    }
    if (route == nullptr) {
      logger.error() << "Failed to create route:" << err->message;
      g_error_free(err);
      continue;
    }

    nm_ip_route_set_attribute(route, NM_IP_ROUTE_ATTRIBUTE_TABLE,
                              g_variant_new_uint32(WG_FIREWALL_MARK));
    nm_ip_route_set_attribute(route, NM_IP_ROUTE_ATTRIBUTE_TYPE,
                              g_variant_new_string("throw"));
    nm_setting_ip_config_add_route(ipcfg, route);
    nm_ip_route_unref(route);
  }

  g_object_ref(m_ipv4config);
  g_object_ref(m_ipv6config);
  nm_connection_add_setting(NM_CONNECTION(m_remote), m_ipv4config);
  nm_connection_add_setting(NM_CONNECTION(m_remote), m_ipv6config);

  // Update the connection settings.
  nm_remote_connection_commit_changes_async(
      m_remote, false, m_cancellable,
      LAMBDA_ASYNC_WRAPPER("peerConfigCompleted"), this);
}

void LinuxNMController::peerConfigCompleted(void* result) {
  GError* err = nullptr;
  gboolean okay = nm_remote_connection_commit_changes_finish(
      m_remote, G_ASYNC_RESULT(result), &err);
  g_object_unref(result);

  if (!okay) {
    logger.error() << "peer configuration failed:" << err->message;
    g_error_free(err);
  } else if (getActiveConnection() != nullptr) {
    logger.debug() << "device already connected";
    emit connected(m_serverPublicKey);
  } else {
    NMConnection* conn = NM_CONNECTION(m_remote);
    const char* ifname = nm_connection_get_interface_name(conn);
    NMDevice* device = nm_client_get_device_by_iface(m_client, ifname);
    logger.debug() << "activating connection:" << ifname;

    nm_client_activate_connection_async(
        m_client, conn, device, nullptr, m_cancellable,
        LAMBDA_ASYNC_WRAPPER("activateCompleted"), this);
  }
}

void LinuxNMController::activateCompleted(void* result) {
  GError* err = nullptr;
  NMActiveConnection* conn = nm_client_activate_connection_finish(
      m_client, G_ASYNC_RESULT(result), &err);

  if (!conn) {
    logger.error() << "peer activation failed:" << err->message;
    g_error_free(err);
  } else {
    emit connected(m_serverPublicKey);
  }

  g_object_unref(conn);
  g_object_unref(result);
}

void LinuxNMController::deactivate(Controller::Reason reason) {
  Q_UNUSED(reason);
  NMActiveConnection* conn = getActiveConnection();

  if (conn == nullptr) {
    logger.warning() << "Client already disconnected";
    emit disconnected();
    return;
  }

  Q_ASSERT(m_client);
  nm_client_deactivate_connection_async(
      m_client, conn, nullptr, LAMBDA_ASYNC_WRAPPER("deactivateCompleted"),
      this);
}

void LinuxNMController::deactivateCompleted(void* result) {
  GError* err = nullptr;
  if (!nm_client_deactivate_connection_finish(m_client, G_ASYNC_RESULT(result),
                                              &err)) {
    logger.error() << "Connection deactivation failed:" << err->message;
    g_error_free(err);
  }
  g_object_unref(result);

  emit disconnected();
}

// TODO: Implement Me!
void LinuxNMController::checkStatus() {
  NMActiveConnection* conn = getActiveConnection();
  if (conn == nullptr) {
    return;
  }

  // At this point, we probably need to go off-script to try and pull
  // the statistics directly using the D-Bus API, since there doesn't
  // appear to be a way to do this directly.
  //
  // TLDR: To get the interface throughput, we need to enable refresh
  // in the statistics interface by setting the RefreshRateMs property
  // and then we can read RxBytes and TxBytes for the data sent and
  // recieved.
  //
  // An ever harder nut to crack without real netlink access. When
  // did the handshake complete? If we can access that data point then
  // we could probably do multihop too.
  const char* objpath = nm_object_get_path(NM_OBJECT(conn));
  logger.debug() << "Checking status of object:" << objpath;

  NMIPConfig* ipv4cfg = nm_active_connection_get_ip4_config(conn);
  if (ipv4cfg == nullptr) {
    return;
  }
  GPtrArray* addrcfg = nm_ip_config_get_addresses(ipv4cfg);
  if (addrcfg->len < 1) {
    return;
  }

  QString deviceAddress(
      nm_ip_address_get_address((NMIPAddress*)(addrcfg->pdata[0])));

  logger.info() << "Status:" << deviceAddress;
  emit statusUpdated(m_serverIpv4Gateway, deviceAddress, 1234500, 6789000);
}

void LinuxNMController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);
  callback("Backend logs are not supported with NetworkManger");
}

void LinuxNMController::cleanupBackendLogs() {}
