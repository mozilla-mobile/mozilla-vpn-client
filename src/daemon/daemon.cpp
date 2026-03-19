/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemon.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>
#include <QTimer>

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "models/server.h"

constexpr const char* JSON_ALLOWEDIPADDRESSRANGES = "allowedIPAddressRanges";
constexpr int HANDSHAKE_POLL_MSEC = 250;

namespace {

Logger logger("Daemon");

Daemon* s_daemon = nullptr;

}  // namespace

Daemon::Daemon(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(Daemon);

  logger.debug() << "Daemon created";

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;

  m_handshakeTimer.setSingleShot(true);
  connect(&m_handshakeTimer, &QTimer::timeout, this, &Daemon::checkHandshake);
}

Daemon::~Daemon() {
  MZ_COUNT_DTOR(Daemon);

  logger.debug() << "Daemon released";

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

// static
Daemon* Daemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
}

bool Daemon::activate(const QString& json) {
  QJsonDocument jsDocument = QJsonDocument::fromJson(json.toUtf8());
  InterfaceConfig ifConfig;

  if (!Daemon::parseConfig(jsDocument.object(), ifConfig)) {
    return false;
  }
  return activate(ifConfig);
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
  // If the activation abort's for any reason `the `activationFailure` signal is
  // emitted.

  logger.debug() << "Activating interface.";
  auto emit_failure_guard = qScopeGuard([this] { emit activationFailure(); });

  if (m_tunnel != nullptr && m_tunnel->connections().contains(config.m_hopType)) {
    if (m_tunnel->supportServerSwitching(config)) {
      logger.debug() << "Already connected. Server switching supported.";

      if (!m_tunnel->switchServer(config)) {
        return false;
      }

      if (!dnsutils()->restoreResolvers()) {
        return false;
      }

      if (!maybeUpdateResolvers(config)) {
        return false;
      }

      bool status = m_tunnel->run(Tunnel::Op::Switch, config);
      logger.debug() << "Connection status after switch:" << status;
      if (status) {
        m_handshakeTimer.start(HANDSHAKE_POLL_MSEC);
        emit_failure_guard.dismiss();
        return true;
      }
      return false;
    }
    else {
      // Server switching is not supported, we need to deactivate and then activate with the new server.
      logger.warning() << "Already connected. Server switching not supported.";
      if (!deactivate(false)) {
        logger.error() << "Deactivation failed during server switch.";
        return false;
      }
    }
  }

  logger.debug() << "Not connected. Activating new connection.";

  if(!selectTunnel(config.m_protocolType)) {
      return false;
  }

  bool status = m_tunnel->run(Tunnel::Op::Up, config);
  logger.debug() << "Connection status after up:" << status;
  if (!maybeUpdateResolvers(config)) {
    return false;
  }
  if (status) {
    m_handshakeTimer.start(HANDSHAKE_POLL_MSEC);
    emit_failure_guard.dismiss();
    return true;
  }
  return false;
}


bool Daemon::maybeUpdateResolvers(const InterfaceConfig& config) {
  logger.debug() << "Maybe update resolvers for" << config.m_hopType;
  if ((config.m_hopType == InterfaceConfig::MultiHopExit) ||
      (config.m_hopType == InterfaceConfig::SingleHop)) {
    QList<QHostAddress> resolvers;
    resolvers.append(QHostAddress(config.m_dnsServer));

    // If the DNS is not the Gateway, it's a user defined DNS
    // thus, not add any other :)
    if (config.m_dnsServer == config.m_serverIpv4Gateway) {
      resolvers.append(QHostAddress(config.m_serverIpv6Gateway));
    }

    if (!dnsutils()->updateResolvers(m_tunnel->interfaceName(), resolvers)) {
      return false;
    }
  }

  return true;
}

// static
bool Daemon::parseStringList(const QJsonObject& obj, const QString& name,
                             QStringList& list) {
  if (obj.contains(name)) {
    QJsonValue value = obj.value(name);
    if (!value.isArray()) {
      logger.error() << name << "is not an array";
      return false;
    }
    QJsonArray array = value.toArray();
    for (const QJsonValue& i : array) {
      if (!i.isString()) {
        logger.error() << name << "must contain only strings";
        return false;
      }
      list.append(i.toString());
    }
  }
  return true;
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

  QMetaEnum protocolMetaEnum = QMetaEnum::fromType<Server::ProtocolType>();
  QJsonValue protocolValue = obj.value("protocolType");
  if (protocolValue.isString()) {
    bool okay;
    config.m_protocolType = Server::ProtocolType(protocolMetaEnum.keyToValue(
        protocolValue.toString().toUtf8().constData(), &okay));
    if (!okay) {
      logger.error() << "protocolType" << protocolValue.toString()
                     << "is not valid";
      return false;
    }
  } else {
    logger.debug() << "protocolType is not a string";
    return false;
  }

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
  
  config.m_hostname = obj.value("hostname").toString();
  if (config.m_hostname.isNull()) {
    logger.warning() << "no server hostname found in jsonConfig input";
    return false;
  }

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

  if (!obj.contains("hopType")) {
    config.m_hopType = InterfaceConfig::SingleHop;
  } else {
    QJsonValue value = obj.value("hopType");
    if (!value.isString()) {
      logger.error() << "hopType is not a string";
      return false;
    }

    bool okay;
    QByteArray vdata = value.toString().toUtf8();
    QMetaEnum meta = QMetaEnum::fromType<InterfaceConfig::HopType>();
    config.m_hopType =
        InterfaceConfig::HopType(meta.keyToValue(vdata.constData(), &okay));
    if (!okay) {
      logger.error() << "hopType" << value.toString() << "is not valid";
      return false;
    }
  }

  if (!obj.contains(JSON_ALLOWEDIPADDRESSRANGES)) {
    logger.error() << JSON_ALLOWEDIPADDRESSRANGES
                   << "missing in the jsonconfig input";
    return false;
  } else {
    QJsonValue value = obj.value(JSON_ALLOWEDIPADDRESSRANGES);
    if (!value.isArray()) {
      logger.error() << JSON_ALLOWEDIPADDRESSRANGES << "is not an array";
      return false;
    }

    QJsonArray array = value.toArray();
    for (const QJsonValue& i : array) {
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

      config.m_allowedIPAddressRanges.append(
          IPAddress(QHostAddress(address.toString()), range.toInt()));
    }

    // Sort allowed IPs by decreasing prefix length.
    std::sort(config.m_allowedIPAddressRanges.begin(),
              config.m_allowedIPAddressRanges.end(),
              [&](const IPAddress& a, const IPAddress& b) -> bool {
                return a.prefixLength() > b.prefixLength();
              });
  }

  if (!parseStringList(obj, "vpnDisabledApps", config.m_vpnDisabledApps)) {
    return false;
  }
  return true;
}

bool Daemon::deactivate(bool emitSignals) {
  Q_ASSERT(m_tunnel != nullptr);
  logger.info() << "Deactivating interface";
  // Deactivate the main interface.
  if (!m_tunnel->connections().isEmpty()) {
    const Tunnel::ConnectionState& state = m_tunnel->connections().first();
    if (!m_tunnel->run(Tunnel::Op::Down, state.m_config)) {
      return false;
    }
  }

  // Emit signals upon return.
  auto guard = qScopeGuard([&]() {
    if (emitSignals) {
      emit disconnected();
    }
  });

  // Cleanup DNS
  if (!dnsutils()->restoreResolvers()) {
    logger.warning() << "Failed to restore DNS resolvers.";
  }

  return m_tunnel->deleteInterface();
}

QString Daemon::logs() {
  QString output;

  {
    QTextStream out(&output);
    LogHandler::instance()->writeLogs(out);
  }

  return output;
}

void Daemon::cleanLogs() { LogHandler::instance()->cleanupLogs(); }

QJsonObject Daemon::getStatus() {
  if(m_tunnel != nullptr) {
    return m_tunnel->getStatus();
  }
  else {
    QJsonObject json;
    json.insert("connected", QJsonValue(false));
    return json;
  }
}

void Daemon::checkHandshake() {
  if(m_tunnel != nullptr) {
    int pendingHandshakes = m_tunnel->checkHandshake();
    if (pendingHandshakes > 0) {
      m_handshakeTimer.start(HANDSHAKE_POLL_MSEC);
    }
  }
}

void Daemon::abortBackendFailure() {
  logger.warning() << "Backend failure occured - disconnecting";
  emit backendFailure();
  deactivate();
}