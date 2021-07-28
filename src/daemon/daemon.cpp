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

// static
bool Daemon::parseConfig(const QJsonObject& obj, InterfaceConfig& config) {
#define GETVALUE(name, where, jsontype)                           \
  if (!obj.contains(name)) {                                      \
    logger.debug() << name << " missing in the jsonConfig input"; \
    return false;                                                 \
  } else {                                                        \
    QJsonValue value = obj.value(name);                           \
    if (value.type() != QJsonValue::jsontype) {                   \
      logger.error() << name << " is not a " #jsontype;           \
      return false;                                               \
    }                                                             \
    where = value.to##jsontype();                                 \
  }

  GETVALUE("privateKey", config.m_privateKey, String);
  GETVALUE("serverPublicKey", config.m_serverPublicKey, String);
  GETVALUE("serverPort", config.m_serverPort, Double);
  GETVALUE("ipv6Enabled", config.m_ipv6Enabled, Bool);

  config.m_deviceIpv4Address = obj.value("deviceIpv4Address").toString();
  config.m_deviceIpv6Address = obj.value("deviceIpv6Address").toString();
  if (config.m_deviceIpv4Address.isNull() &&
      config.m_deviceIpv6Address.isNull()) {
    logger.warning() << "no device addresses found in jsonConfig input";
    return false;
  }
  config.m_serverIpv4AddrIn = obj.value("serverIpv4AddrIn").toString();
  config.m_serverIpv6AddrIn = obj.value("serverIpv6AddrIn").toString();
  if (config.m_serverIpv4AddrIn.isNull() &&
      config.m_serverIpv6AddrIn.isNull()) {
    logger.error() << "no server addresses found in jsonConfig input";
    return false;
  }
  config.m_serverIpv4Gateway = obj.value("serverIpv4Gateway").toString();
  config.m_serverIpv6Gateway = obj.value("serverIpv6Gateway").toString();

  if (!obj.contains("dnsServer")) {
    config.m_dnsServer = QString();
  } else {
    QJsonValue value = obj.value("dnsServer");
    if (!value.isString()) {
      logger.error() << "dnsServer is not a string";
      return false;
    }
    config.m_dnsServer = value.toString();
  }

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

bool Daemon::deactivate(bool emitSignals) {
  // Cleanup DNS
  if (supportDnsUtils() && !dnsutils()->restoreResolvers()) {
    return false;
  }

  if (supportWGUtils()) {
    if (!wgutils()->interfaceExists()) {
      logger.warning() << "Wireguard interface does not exist.";
      return false;
    }

    // Cleanup routing
    for (const ConnectionState& state : m_connections.values()) {
      const InterfaceConfig& config = state.m_config;
      logger.debug() << "Deleting routes for hop" << config.m_hopindex;
      for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
        wgutils()->deleteRoutePrefix(ip, config.m_hopindex);
      }
    }

    // Delete the interface
    if (!wgutils()->deleteInterface()) {
      return false;
    }
  }

  // Deactivate the main interface.
  if (m_connections.contains(0)) {
    const ConnectionState& state = m_connections.value(0);
    if (!run(Down, state.m_config)) {
      return false;
    }
    if (emitSignals) {
      emit disconnected(0);
    }
  }

  m_connections.clear();
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

  // Activate the new peer and its routes.
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

  // Deactivate the old peer and remove its routes.
  for (const IPAddressRange& ip : lastConfig.m_allowedIPAddressRanges) {
    if (!config.m_allowedIPAddressRanges.contains(ip)) {
      wgutils()->deleteRoutePrefix(ip, config.m_hopindex);
    }
  }
  if (!wgutils()->deletePeer(lastConfig.m_serverPublicKey)) {
    return false;
  }

  m_connections[config.m_hopindex] = ConnectionState(config);
  return true;
}
