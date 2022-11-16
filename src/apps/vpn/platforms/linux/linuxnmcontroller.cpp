/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxnmcontroller.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"

#include <QScopeGuard>

#if defined(__cplusplus)
extern "C" {
#endif

#include <glib.h>

#undef signals
#include <NetworkManager.h>
#define signals Q_SIGNALS

#if defined(__cplusplus)
};
#endif

constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
constexpr const char* WG_INTERFACE_NAME = "wg0";

namespace {
Logger logger({LOG_LINUX, LOG_CONTROLLER}, "LinuxNMController");
}

LinuxNMController::LinuxNMController() {
  MVPN_COUNT_CTOR(LinuxNMController);

  GError *err = nullptr;
  m_client = nm_client_new(nullptr, &err);
  if (m_client) {
    logger.info() << "NetworkManager version:" << nm_client_get_version(m_client);
  } else {
    logger.error() << "Failed to create NetworkManager client:" << err->message;
  }

  m_cancellable = g_cancellable_new();
}

LinuxNMController::~LinuxNMController() {
  MVPN_COUNT_DTOR(LinuxNMController);
  logger.debug() << "Destroying LinuxNMController";

  if (m_connection) {
    nm_remote_connection_delete_async(m_connection, nullptr,
        [](GObject* obj, GAsyncResult *res, gpointer data){
          Q_UNUSED(obj);
          GError *err = nullptr;
          if (!nm_remote_connection_delete_finish(NM_REMOTE_CONNECTION(data),
                                                  res, &err)) {
            logger.error() << "Connection deletion failed:" << err->message;
          }
        }, m_connection);
  }

  g_cancellable_cancel(m_cancellable);
  g_object_unref(m_cancellable);
  g_object_unref(m_client);
}

void LinuxNMController::initializeCompleted(GObject* obj, GAsyncResult* res, void *data) {
  LinuxNMController* controller = static_cast<LinuxNMController*>(data);
  GError *err = nullptr;
  GVariant *gv;

  controller->m_connection =
      nm_client_add_connection2_finish(controller->m_client, res, &gv, &err);
  if(!controller->m_connection) {
    logger.error() << "connection creation failed:" << err->message;
  }
}

static NMIPAddress* parseAddress(const QString& addr) {
  uint prefix = -1;
  int slash = addr.indexOf('/');
  QString base = addr.first(slash);

  if (slash > 0) {
    prefix = addr.mid(slash+1).toUInt();
  }

  if (base.contains(':')) {
    if (prefix < 0) prefix = 128;
    return nm_ip_address_new(AF_INET6, qPrintable(base), prefix, nullptr);
  } else {
    if (prefix < 0) prefix = 32;
    return nm_ip_address_new(AF_INET, qPrintable(base), prefix, nullptr);
  }
}

void LinuxNMController::initialize(const Device* device, const Keys* keys) {
  NMConnection* wg_connection = nm_simple_connection_new();

  // Generic connection settings.
  NMSettingConnection* scon = NM_SETTING_CONNECTION(nm_setting_connection_new());
  nm_connection_add_setting(wg_connection, NM_SETTING(scon));
  g_object_set(scon,
               NM_SETTING_CONNECTION_ID,
               WG_INTERFACE_NAME,
               NM_SETTING_CONNECTION_TYPE,
               NM_SETTING_WIREGUARD_SETTING_NAME,
               NM_SETTING_CONNECTION_INTERFACE_NAME,
               WG_INTERFACE_NAME,
               NULL);

  // Address settings
  NMSettingIPConfig* ipv4 = NM_SETTING_IP_CONFIG(nm_setting_ip4_config_new());
  g_object_set(ipv4,
               NM_SETTING_IP_CONFIG_METHOD,
               NM_SETTING_IP4_CONFIG_METHOD_MANUAL,
               NULL);
  nm_connection_add_setting(wg_connection, NM_SETTING(ipv4));
  nm_setting_ip_config_add_address(ipv4, parseAddress(device->ipv4Address()));

  NMSettingIPConfig* ipv6 = NM_SETTING_IP_CONFIG(nm_setting_ip6_config_new());
  g_object_set(ipv6,
               NM_SETTING_IP_CONFIG_METHOD,
               NM_SETTING_IP6_CONFIG_METHOD_MANUAL,
               NULL);
  nm_connection_add_setting(wg_connection, NM_SETTING(ipv6));
  nm_setting_ip_config_add_address(ipv6, parseAddress(device->ipv6Address()));

  // Wireguard connection settings.
  NMSettingWireGuard* wg = NM_SETTING_WIREGUARD(nm_setting_wireguard_new());
  nm_connection_add_setting(wg_connection, NM_SETTING(wg));
  g_object_set(wg,
               NM_SETTING_WIREGUARD_PRIVATE_KEY,
               qPrintable(keys->privateKey()),
               NM_SETTING_WIREGUARD_FWMARK,
               WG_FIREWALL_MARK,
               NULL);

  // Add the connection
  logger.info() << "Creating connection";
  nm_client_add_connection2(
    m_client,
    nm_connection_to_dbus(wg_connection, NM_CONNECTION_SERIALIZE_ALL),
    NM_SETTINGS_ADD_CONNECTION2_FLAG_IN_MEMORY,
    nullptr,
    false,
    m_cancellable,
    &LinuxNMController::initializeCompleted,
    this
  );
}

void LinuxNMController::activate(const HopConnection& hop, const Device* device,
                               const Keys* keys, Reason reason) {
  Q_UNUSED(device);
  Q_UNUSED(keys);
  Q_UNUSED(reason);

  logger.debug() << "LinuxNMController activated" << hop.m_server.hostname();
  logger.debug() << "LinuxNMController DNS" << hop.m_dnsServer.toString();

  //m_connected = true;
  //m_publicKey = hop.m_server.publicKey();
  //m_delayTimer.start(DUMMY_CONNECTION_DELAY_MSEC);
}

void LinuxNMController::deactivate(Reason reason) {
  Q_UNUSED(reason);

  logger.debug() << "LinuxNMController deactivated";

  //m_connected = false;
  //m_publicKey.clear();
  //m_delayTimer.start(DUMMY_CONNECTION_DELAY_MSEC);
}

// TODO: Implement Me!
void LinuxNMController::checkStatus() {
  emit statusUpdated("127.0.0.1", "127.0.0.1", 1234500, 6789000);
}

void LinuxNMController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);
  callback("Backend logs are not supported with NetworkManger");
}

void LinuxNMController::cleanupBackendLogs() { /* nothing to do here */ }
