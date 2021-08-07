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

  if (m_connected) {
    if (supportServerSwitching(config)) {
      logger.debug() << "Already connected. Server switching supported.";

      m_lastConfig = config;
      if (!switchServer(config)) {
        return false;
      }

      m_connectionDate = QDateTime::currentDateTime();
      emit connected();
      return true;
    }

    logger.warning() << "Already connected. Server switching not supported.";
    if (!deactivate(false)) {
      return false;
    }

    Q_ASSERT(!m_connected);
    return activate(config);
  }

  prepareActivation(config);

  if (supportWGUtils()) {
    if (wgutils()->interfaceExists()) {
      qWarning("Wireguard interface `%s` already exists.", WG_INTERFACE);
      return false;
    }
    // add_if and configure
    if (!wgutils()->addInterface(config)) {
      qWarning("Interface creation failed. Removing `%s`.", WG_INTERFACE);
      return false;
    }
  }
  if (supportDnsUtils()) {
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
    if (!iputils()->setMTUAndUp()) {
      return false;
    }
  }
  if (supportWGUtils()) {
    // set routing
    for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
      if (!wgutils()->addRoutePrefix(ip)) {
        qWarning("Routing configuration failed. Removing `%s`.", WG_INTERFACE);
        return false;
      }
    }
  }

  m_lastConfig = config;
  m_connected = run(Up, m_lastConfig);

  logger.debug() << "Connection status:" << m_connected;

  if (m_connected) {
    m_connectionDate = QDateTime::currentDateTime();
    emit connected();
  }

  return m_connected;
}

// static
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
  logger.debug() << "Deactivate";

  if (!m_connected) {
    logger.error() << "Already disconnected";
    return true;
  }

  if (supportDnsUtils()) {
    if (!dnsutils()->restoreResolvers()) {
      return false;
    }
  }

  if (supportWGUtils() && !wgutils()->interfaceExists()) {
    qWarning("Wireguard interface `%s` does not exist. Cannot proceed.",
             WG_INTERFACE);
    return false;
  }

  m_connected = false;
  bool status = run(Down, m_lastConfig);

  if (supportWGUtils() && !wgutils()->deleteInterface()) {
    return false;
  }

  logger.debug() << "Status:" << status;

  // No notification for server switching.
  if (emitSignals && status) {
    emit disconnected();
  }

  return status;
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

  logger.debug() << "Switching server";

  Q_ASSERT(m_connected);
  wgutils()->flushRoutes();

  if (!wgutils()->updateInterface(config)) {
    logger.error() << "Server switch failed to update the wireguard interface";
    return false;
  }

  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    if (!wgutils()->addRoutePrefix(ip)) {
      logger.error() << "Server switch failed to update the routing table";
      return false;
    }
  }

  return true;
}
