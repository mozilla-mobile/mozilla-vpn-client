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

  logger.log() << "Daemon created";

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;
}

Daemon::~Daemon() {
  MVPN_COUNT_DTOR(Daemon);

  logger.log() << "Daemon released";

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

// static
Daemon* Daemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
}

bool Daemon::activate(const Config& config) {
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
      logger.log() << "Already connected. Server switching supported.";

      m_lastConfig = config;
      if (!switchServer(config)) {
        return false;
      }

      m_connectionDate = QDateTime::currentDateTime();
      emit connected();
      return true;
    }

    logger.log() << "Already connected. Server switching not supported.";
    if (!deactivate(false)) {
      return false;
    }

    Q_ASSERT(!m_connected);
    return activate(config);
  }

  m_lastConfig = config;
  m_connected = run(Up, m_lastConfig);

  logger.log() << "Connection status:" << m_connected;

  if (m_connected) {
    m_connectionDate = QDateTime::currentDateTime();
    emit connected();
  }

  return m_connected;
}

// static
bool Daemon::parseConfig(const QJsonObject& obj, Config& config) {
#define GETVALUESTR(name, where)                                \
  if (!obj.contains(name)) {                                    \
    logger.log() << name << " missing in the jsonConfig input"; \
    return false;                                               \
  }                                                             \
  {                                                             \
    QJsonValue value = obj.value(name);                         \
    if (!value.isString()) {                                    \
      logger.log() << name << " is not a string";               \
      return false;                                             \
    }                                                           \
    where = value.toString();                                   \
  }

  GETVALUESTR("privateKey", config.m_privateKey);
  GETVALUESTR("deviceIpv4Address", config.m_deviceIpv4Address);
  GETVALUESTR("deviceIpv6Address", config.m_deviceIpv6Address);
  GETVALUESTR("serverIpv4Gateway", config.m_serverIpv4Gateway);
  GETVALUESTR("serverIpv6Gateway", config.m_serverIpv6Gateway);
  GETVALUESTR("serverPublicKey", config.m_serverPublicKey);
  GETVALUESTR("serverIpv4AddrIn", config.m_serverIpv4AddrIn);
  GETVALUESTR("serverIpv6AddrIn", config.m_serverIpv6AddrIn);

#undef GETVALUESTR

#define GETVALUEINT(name, where)                                \
  if (!obj.contains(name)) {                                    \
    logger.log() << name << " missing in the jsonConfig input"; \
    return false;                                               \
  }                                                             \
  {                                                             \
    QJsonValue value = obj.value(name);                         \
    if (!value.isDouble()) {                                    \
      logger.log() << name << " is not a number";               \
      return false;                                             \
    }                                                           \
    where = value.toInt();                                      \
  }

  GETVALUEINT("serverPort", config.m_serverPort);

#undef GETVALUEINT

#define GETVALUEBOOL(name, where)                               \
  if (!obj.contains(name)) {                                    \
    logger.log() << name << " missing in the jsonConfig input"; \
    return false;                                               \
  }                                                             \
  {                                                             \
    QJsonValue value = obj.value(name);                         \
    if (!value.isBool()) {                                      \
      logger.log() << name << " is not a boolean";              \
      return false;                                             \
    }                                                           \
    where = value.toBool();                                     \
  }

  GETVALUEBOOL("ipv6Enabled", config.m_ipv6Enabled);

#undef GETVALUEBOOL

  if (!obj.contains(JSON_ALLOWEDIPADDRESSRANGES)) {
    logger.log() << JSON_ALLOWEDIPADDRESSRANGES
                 << "missing in the jsonconfig input";
    return false;
  }
  {
    QJsonValue value = obj.value(JSON_ALLOWEDIPADDRESSRANGES);
    if (!value.isArray()) {
      logger.log() << JSON_ALLOWEDIPADDRESSRANGES << "is not an array";
      return false;
    }

    QJsonArray array = value.toArray();
    for (QJsonValue i : array) {
      if (!i.isObject()) {
        logger.log() << JSON_ALLOWEDIPADDRESSRANGES
                     << "must contain only objects";
        return false;
      }

      QJsonObject ipObj = i.toObject();

      QJsonValue address = ipObj.value("address");
      if (!address.isString()) {
        logger.log() << JSON_ALLOWEDIPADDRESSRANGES
                     << "objects must have a string address";
        return false;
      }

      QJsonValue range = ipObj.value("range");
      if (!range.isDouble()) {
        logger.log() << JSON_ALLOWEDIPADDRESSRANGES
                     << "object must have a numberic range";
        return false;
      }

      QJsonValue isIpv6 = ipObj.value("isIpv6");
      if (!isIpv6.isBool()) {
        logger.log() << JSON_ALLOWEDIPADDRESSRANGES
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

  return true;
}

bool Daemon::deactivate(bool emitSignals) {
  logger.log() << "Deactivate";

  if (!m_connected) {
    logger.log() << "Already disconnected";
    return true;
  }

  m_connected = false;

  bool status = run(Down, m_lastConfig);

  logger.log() << "Status:" << status;

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

bool Daemon::switchServer(const Config& config) {
  Q_UNUSED(config);
  qFatal("Have you forgotten to implement switchServer?");
  return false;
}
