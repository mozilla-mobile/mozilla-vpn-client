/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

constexpr const char* JSON_ALLOWEDIPADDRESSRANGES = "allowedIPAddressRanges";

namespace {

Logger logger(LOG_MAIN, "Daemon");

Daemon* s_daemon = nullptr;

}  // namespace

Daemon::Daemon(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(Daemon);

  logger.debug() << "Daemon created";

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;
}

Daemon::~Daemon() {
  MVPN_COUNT_DTOR(Daemon);

  logger.debug() << "Daemon released";

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

// static
Daemon* Daemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
}

bool Daemon::activate(const InterfaceConfig& config) {
  // There are 3 possible scenarios in which this method is called:
  //
  // 1. the VPN is off: the method tries to enable the VPN.
  // 2. the VPN is on and the platform doesn't support the server-switching:
  //    this method calls deactivate() and then it continues as 1.
  // 3. the VPN is on and the platform supports the server-switching: this
  //    method calls switchServer().
  //
  // At the end, if the activation succeds, the `connected` signal is emitted.
  logger.debug() << "Activating interface";

  if (m_connections.contains(config.m_hopindex)) {
    if (supportServerSwitching(config)) {
      logger.debug() << "Already connected. Server switching supported.";

      if (!switchServer(config)) {
        return false;
      }
      m_connections[config.m_hopindex] = ConnectionState(config);

      emit connected(config.m_hopindex);
      return true;
    }

    logger.warning() << "Already connected. Server switching not supported.";
    if (!deactivate(false)) {
      return false;
    }

    Q_ASSERT(!m_connections.contains(config.m_hopindex));
    return activate(config);
  }

  prepareActivation(config);

  if (supportWGUtils()) {
    // Bring up the wireguard interface if not already done.
    if (!wgutils()->interfaceExists()) {
      if (!wgutils()->addInterface(config)) {
        logger.error() << "Interface creation failed.";
        return false;
      }
    }
    // Add the peer to this interface.
    if (!wgutils()->updatePeer(config)) {
      logger.error() << "Peer creation failed.";
      return false;
    }
  }

  if ((config.m_hopindex == 0) && supportDnsUtils()) {
    QList<QHostAddress> resolvers;
    resolvers.append(QHostAddress(config.m_dnsServer));

    // If the DNS is not the Gateway, it's a user defined DNS
    // thus, not add any other :)
    if (config.m_ipv6Enabled &&
        config.m_dnsServer == config.m_serverIpv4Gateway) {
      resolvers.append(QHostAddress(config.m_serverIpv6Gateway));
    }

    if (!dnsutils()->updateResolvers(WG_INTERFACE, resolvers)) {
      return false;
    }
  }

  if (supportIPUtils()) {
    if (!iputils()->addInterfaceIPs(config)) {
      return false;
    }
    if (!iputils()->setMTUAndUp(config)) {
      return false;
    }
  }

  // set routing
  if (supportWGUtils()) {
    for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
      if (!wgutils()->updateRoutePrefix(ip, config.m_hopindex)) {
        logger.debug() << "Routing configuration failed for" << ip.toString();
        return false;
      }
    }
  }

  bool status = run(Up, config);
  logger.debug() << "Connection status:" << status;
  if (status) {
    m_connections[config.m_hopindex] = ConnectionState(config);
    emit connected(config.m_hopindex);
  }

  return status;
}

bool Daemon::parseConfig(const QJsonObject& obj, InterfaceConfig& config) {
#define GETVALUESTR(name, where)                                  \
  if (!obj.contains(name)) {                                      \
    logger.debug() << name << " missing in the jsonConfig input"; \
    return false;                                                 \
  }                                                               \
  {                                                               \
    QJsonValue value = obj.value(name);                           \
    if (!value.isString()) {                                      \
      logger.error() << name << " is not a string";               \
      return false;                                               \
    }                                                             \
    where = value.toString();                                     \
  }

  GETVALUESTR("privateKey", config.m_privateKey);
  GETVALUESTR("deviceIpv4Address", config.m_deviceIpv4Address);
  GETVALUESTR("deviceIpv6Address", config.m_deviceIpv6Address);
  GETVALUESTR("serverIpv4Gateway", config.m_serverIpv4Gateway);
  GETVALUESTR("serverIpv6Gateway", config.m_serverIpv6Gateway);
  GETVALUESTR("serverPublicKey", config.m_serverPublicKey);
  GETVALUESTR("serverIpv4AddrIn", config.m_serverIpv4AddrIn);
  GETVALUESTR("serverIpv6AddrIn", config.m_serverIpv6AddrIn);
  GETVALUESTR("dnsServer", config.m_dnsServer);

#undef GETVALUESTR

#define GETVALUEINT(name, where)                                  \
  if (!obj.contains(name)) {                                      \
    logger.debug() << name << " missing in the jsonConfig input"; \
    return false;                                                 \
  }                                                               \
  {                                                               \
    QJsonValue value = obj.value(name);                           \
    if (!value.isDouble()) {                                      \
      logger.error() << name << " is not a number";               \
      return false;                                               \
    }                                                             \
    where = value.toInt();                                        \
  }

  GETVALUEINT("serverPort", config.m_serverPort);

#undef GETVALUEINT

#define GETVALUEBOOL(name, where)                                 \
  if (!obj.contains(name)) {                                      \
    logger.debug() << name << " missing in the jsonConfig input"; \
    return false;                                                 \
  }                                                               \
  {                                                               \
    QJsonValue value = obj.value(name);                           \
    if (!value.isBool()) {                                        \
      logger.error() << name << " is not a boolean";              \
      return false;                                               \
    }                                                             \
    where = value.toBool();                                       \
  }

  GETVALUEBOOL("ipv6Enabled", config.m_ipv6Enabled);

#undef GETVALUEBOOL

  if (!obj.contains("hopindex")) {
    config.m_hopindex = 0;
  } else {
    QJsonValue value = obj.value("hopindex");
    if (!value.isDouble()) {
      logger.error() << "hopindex is not a number";
      return false;
    }
    config.m_hopindex = value.toInt();
  }

  if (!obj.contains(JSON_ALLOWEDIPADDRESSRANGES)) {
    logger.error() << JSON_ALLOWEDIPADDRESSRANGES
                   << "missing in the jsonconfig input";
    return false;
  }
  {
    QJsonValue value = obj.value(JSON_ALLOWEDIPADDRESSRANGES);
    if (!value.isArray()) {
      logger.error() << JSON_ALLOWEDIPADDRESSRANGES << "is not an array";
      return false;
    }

    QJsonArray array = value.toArray();
    for (QJsonValue i : array) {
      if (!i.isObject()) {
        logger.error() << JSON_ALLOWEDIPADDRESSRANGES
                       << "must contain only objects";
        return false;
      }

      QJsonObject ipObj = i.toObject();

      QJsonValue address = ipObj.value("address");
      if (!address.isString()) {
        logger.error() << JSON_ALLOWEDIPADDRESSRANGES
                       << "objects must have a string address";
        return false;
      }

      QJsonValue range = ipObj.value("range");
      if (!range.isDouble()) {
        logger.error() << JSON_ALLOWEDIPADDRESSRANGES
                       << "object must have a numberic range";
        return false;
      }

      QJsonValue isIpv6 = ipObj.value("isIpv6");
      if (!isIpv6.isBool()) {
        logger.error() << JSON_ALLOWEDIPADDRESSRANGES
                       << "object must have a boolean isIpv6";
        return false;
      }

      if (isIpv6.toBool() && !config.m_ipv6Enabled) {
        continue;
      }

      config.m_allowedIPAddressRanges.append(IPAddressRange(
          address.toString(), range.toInt(),
          isIpv6.toBool() ? IPAddressRange::IPv6 : IPAddressRange::IPv4));
    }
  }

  {  // Read Split Tunnel Apps
    QJsonValue value = obj.value("vpnDisabledApps");
    if (!value.isArray()) {
      logger.error() << "vpnDisabledApps is not an array";
      return false;
    }
    QJsonArray array = value.toArray();
    for (QJsonValue i : array) {
      if (!i.isString()) {
        logger.error() << "vpnDisabledApps must contain only strings";
        return false;
      }
      config.m_vpnDisabledApps.append(i.toString());
    }
  }
  return true;
}

bool Daemon::deactivate(int hopindex, bool emitSignals) {
  logger.debug() << "Deactivating hop" << hopindex;

  if (!m_connections.contains(hopindex)) {
    logger.error() << "Wireguard interface hop" << hopindex << "does not exist.";
    return false;
  }
  const InterfaceConfig& config = m_connections.value(hopindex).m_config;

  // For multihop interfaces, just remove the peer and carry on.
  if (hopindex > 0) {
    Q_ASSERT(supportWGUtils());
    if (!wgutils()->deletePeer(config.m_serverPublicKey)) {
      return false;
    }
    // TODO: Any routing to clean up?
  }
  // Otherwise, we are deactivating the primary endpoint.
  else {
    if (supportDnsUtils() && !dnsutils()->restoreResolvers()) {
      return false;
    }
  }
  m_connections.remove(hopindex);

  bool status = run(Down, config);

  // Deactivate the wireguard interface if there are no more connections.
  if (m_connections.isEmpty() && supportWGUtils()) {
    if (!wgutils()->interfaceExists()) {
      logger.warning() << "Wireguard interface does not exist.";
      return false;
    }
    if (!wgutils()->deleteInterface()) {
      return false;
    }
  }

  // No notification for server switching.
  if (emitSignals && status) {
    emit disconnected(hopindex);
  }

  return status;
}

bool Daemon::deactivateAll(bool emitSignals) {
  for (int hopindex : m_connections.keys()) {
    deactivate(hopindex, emitSignals);
  }
  return true;
}

QString Daemon::logs() {
  QString output;

  {
    QTextStream out(&output);
    LogHandler::writeLogs(out);
  }

  return output;
}

void Daemon::cleanLogs() { LogHandler::instance()->cleanupLogs(); }

bool Daemon::switchServer(const InterfaceConfig& config) {
  // Generic server switching is not supported without wgutils.
  if (!supportWGUtils()) {
    qFatal("Have you forgotten to implement switchServer?");
    return false;
  }

  logger.debug() << "Switching server for hop" << config.m_hopindex;

  Q_ASSERT(m_connections.contains(config.m_hopindex));
  const InterfaceConfig& lastConfig =
      m_connections.value(config.m_hopindex).m_config;

  // Activate the new peer.
  if (!wgutils()->updatePeer(config)) {
    logger.error() << "Server switch failed to update the wireguard interface";
    return false;
  }
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    if (!wgutils()->updateRoutePrefix(ip, config.m_hopindex)) {
      logger.error() << "Server switch failed to update the routing table";
      break;
    }
  }

  // Deactivate the old peer and remove any stale routes.
  if (!wgutils()->deletePeer(lastConfig.m_serverPublicKey)) {
    return false;
  }
  for (const IPAddressRange& ip : lastConfig.m_allowedIPAddressRanges) {
    if (!config.m_allowedIPAddressRanges.contains(ip)) {
      wgutils()->deleteRoutePrefix(ip, config.m_hopindex);
    }
  }

  m_connections[config.m_hopindex] = ConnectionState(config);
  return true;
}
