/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbusservice.h"
#include "dbus_adaptor.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "polkithelper.h"
#include "wgquickprocess.h"
#include "wgutilslinux.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"

#if defined(__cplusplus)
}
#endif

namespace {
Logger logger(LOG_LINUX, "DBusService");
}

DBusService::DBusService(QObject* parent) : Daemon(parent) {
  MVPN_COUNT_CTOR(DBusService);
}

DBusService::~DBusService() { MVPN_COUNT_DTOR(DBusService); }

WireguardUtilsLinux* DBusService::wgutils() {
  if (!m_wgutils) {
    m_wgutils = new WireguardUtilsLinux();
  }
  return m_wgutils;
}

void DBusService::setAdaptor(DbusAdaptor* adaptor) {
  Q_ASSERT(!m_adaptor);
  m_adaptor = adaptor;
}

bool DBusService::removeInterfaceIfExists() {
  return wgutils()->removeInterfaceIfExists();
}

QString DBusService::version() {
  logger.log() << "Version request";
  return PROTOCOL_VERSION;
}

bool DBusService::activate(const QString& jsonConfig) {
  logger.log() << "Activate";

  if (!PolkitHelper::instance()->checkAuthorization(
          "org.mozilla.vpn.activate")) {
    logger.log() << "Polkit rejected";
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(jsonConfig.toLocal8Bit());
  if (!json.isObject()) {
    logger.log() << "Invalid input";
    return false;
  }

  QJsonObject obj = json.object();

  Config config;
  if (!parseConfig(obj, config)) {
    logger.log() << "Invalid configuration";
    return false;
  }

  return Daemon::activate(config);
}

bool DBusService::deactivate(bool emitSignals) {
  logger.log() << "Deactivate";
  return Daemon::deactivate(emitSignals);
}

QString DBusService::status() { return QString(getStatus()); }

QByteArray DBusService::getStatus() {
  logger.log() << "Status request";

  QJsonObject json;

  wg_device* device = nullptr;
  if (wg_get_device(&device, WG_INTERFACE) != 0) {
    logger.log() << "Unable to get device";
    json.insert("status", QJsonValue(false));
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
  }

  uint64_t txBytes = 0;
  uint64_t rxBytes = 0;

  wg_peer* peer;
  wg_for_each_peer(device, peer) {
    txBytes += peer->tx_bytes;
    rxBytes += peer->rx_bytes;
  }

  wg_free_device(device);

  json.insert("status", QJsonValue(true));
  json.insert("serverIpv4Gateway",
              QJsonValue(m_lastConfig.m_serverIpv4Gateway));
  json.insert("txBytes", QJsonValue(double(txBytes)));
  json.insert("rxBytes", QJsonValue(double(rxBytes)));

  return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

QString DBusService::getLogs() {
  logger.log() << "Log request";
  return Daemon::logs();
}

bool DBusService::run(Op op, const Config& config) {
  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  return WgQuickProcess::run(
      op, config.m_privateKey, config.m_deviceIpv4Address,
      config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
      config.m_serverIpv6Gateway, config.m_serverPublicKey,
      config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
      addresses.join(", "), config.m_serverPort, config.m_ipv6Enabled);
}

static inline bool endpointStringToSockaddr(const QString& address, int port,
                                            struct sockaddr* endpoint) {
  QString portString = QString::number(port);

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  struct addrinfo* resolved = nullptr;
  int retries = 15;

  for (unsigned int timeout = 1000000;;
       timeout = std::min((unsigned int)20000000, timeout * 6 / 5)) {
    int rv = getaddrinfo(address.toLocal8Bit(), portString.toLocal8Bit(),
                         &hints, &resolved);
    if (!rv) {
      break;
    }

    /* The set of return codes that are "permanent failures". All other
     * possibilities are potentially transient.
     *
     * This is according to https://sourceware.org/glibc/wiki/NameResolver which
     * states: "From the perspective of the application that calls getaddrinfo()
     * it perhaps doesn't matter that much since EAI_FAIL, EAI_NONAME and
     * EAI_NODATA are all permanent failure codes and the causes are all
     * permanent failures in the sense that there is no point in retrying
     * later."
     *
     * So this is what we do, except FreeBSD removed EAI_NODATA some time ago,
     * so that's conditional.
     */
    if (rv == EAI_NONAME || rv == EAI_FAIL ||
#ifdef EAI_NODATA
        rv == EAI_NODATA ||
#endif
        (retries >= 0 && !retries--)) {
      logger.log() << "Failed to resolve the address endpoint";
      return false;
    }

    logger.log() << "Trying again in" << (timeout / 1000000.0) << "seconds";
    usleep(timeout);
  }

  if ((resolved->ai_family == AF_INET &&
       resolved->ai_addrlen == sizeof(struct sockaddr_in)) ||
      (resolved->ai_family == AF_INET6 &&
       resolved->ai_addrlen == sizeof(struct sockaddr_in6))) {
    memcpy(endpoint, resolved->ai_addr, resolved->ai_addrlen);
    freeaddrinfo(resolved);
    return true;
  }

  logger.log() << "Invalid endpoint" << address;
  freeaddrinfo(resolved);
  return false;
}

bool DBusService::switchServer(const Config& config) {
  logger.log() << "Switching server";

  wg_device* device = static_cast<wg_device*>(calloc(1, sizeof(*device)));
  if (!device) {
    logger.log() << "Allocation failure";
    return false;
  }

  auto guard = qScopeGuard([&] { wg_free_device(device); });

  strncpy(device->name, WG_INTERFACE, IFNAMSIZ - 1);
  device->name[IFNAMSIZ - 1] = '\0';

  wg_peer* peer = static_cast<wg_peer*>(calloc(1, sizeof(*peer)));
  if (!peer) {
    logger.log() << "Allocation failure";
    return false;
  }

  peer->flags =
      (wg_peer_flags)(WGPEER_HAS_PUBLIC_KEY | WGPEER_REPLACE_ALLOWEDIPS);

  device->first_peer = device->last_peer = peer;
  device->flags = WGDEVICE_REPLACE_PEERS;

  if (!endpointStringToSockaddr(config.m_serverIpv4AddrIn, config.m_serverPort,
                                &peer->endpoint.addr)) {
    return false;
  }

  wg_key_from_base64(peer->public_key, config.m_serverPublicKey.toLocal8Bit());

  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    wg_allowedip* allowedip =
        static_cast<wg_allowedip*>(calloc(1, sizeof(*allowedip)));
    if (!allowedip) {
      logger.log() << "Allocation failure";
      return false;
    }

    if (!peer->first_allowedip) {
      peer->first_allowedip = allowedip;
    } else {
      peer->last_allowedip->next_allowedip = allowedip;
    }

    peer->last_allowedip = allowedip;
    allowedip->cidr = ip.range();

    bool ok = false;
    if (ip.type() == IPAddressRange::IPv4) {
      allowedip->family = AF_INET;
      ok = inet_pton(AF_INET, ip.ipAddress().toLocal8Bit(), &allowedip->ip4) ==
           1;
    } else if (ip.type() == IPAddressRange::IPv6) {
      allowedip->family = AF_INET6;
      ok = inet_pton(AF_INET6, ip.ipAddress().toLocal8Bit(), &allowedip->ip6) ==
           1;
    } else {
      logger.log() << "Invalid IPAddressRange type";
      return false;
    }

    if (!ok) {
      logger.log() << "Invalid IP address:" << ip.ipAddress();
      return false;
    }
  }

  if (wg_set_device(device) != 0) {
    logger.log() << "Failed to set the new peer";
    return false;
  }

  return true;
}

bool DBusService::supportServerSwitching(const Config& config) const {
  return m_lastConfig.m_privateKey == config.m_privateKey &&
         m_lastConfig.m_deviceIpv4Address == config.m_deviceIpv4Address &&
         m_lastConfig.m_deviceIpv6Address == config.m_deviceIpv6Address &&
         m_lastConfig.m_serverIpv4Gateway == config.m_serverIpv4Gateway &&
         m_lastConfig.m_serverIpv6Gateway == config.m_serverIpv6Gateway;
}
