/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "interfaceconfig.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>
#include <QScopeGuard>

#include "logger.h"

namespace {
Logger logger("InterfaceConfig");
}  // namespace

constexpr const char* JSON_ALLOWED_ADDRESS_RANGES = "allowedIPAddressRanges";

InterfaceConfig::InterfaceConfig(const QJsonObject& json) {
  // If anything goes wrong during parsing, make sure isValid() returns false.
  auto guard = qScopeGuard([this]() {
    m_privateKey.clear();
    m_publicKey.clear();
    m_deviceIpv4Address.clear();
    m_deviceIpv6Address.clear();
  });

  // Parse the device configuration.
  if (!getRequiredString(json, "privateKey", m_privateKey)) return;
  if (!getRequiredString(json, "publicKey", m_publicKey)) return;
  m_deviceIpv4Address = json.value("deviceIpv4Address").toString();
  m_deviceIpv6Address = json.value("deviceIpv6Address").toString();
  if (m_deviceIpv4Address.isEmpty() && m_deviceIpv6Address.isEmpty()) {
    logger.warning() << "no device addresses found in json input";
    return;
  }

  // Parse the wireguard configuration.
  if (!getRequiredString(json, "serverPublicKey", m_serverPublicKey)) return;
  m_serverPort = json.value("serverPort").toInt(51820);
  m_serverIpv4AddrIn = json.value("serverIpv4AddrIn").toString();
  m_serverIpv6AddrIn = json.value("serverIpv6AddrIn").toString();
  if (m_serverIpv4AddrIn.isEmpty() && m_serverIpv6AddrIn.isEmpty()) {
    logger.warning() << "no server addresses found in json input";
    return;
  }
  m_serverIpv4Gateway = json.value("serverIpv4Gateway").toString();
  m_serverIpv6Gateway = json.value("serverIpv6Gateway").toString();

  if (!json.contains("dnsServer")) {
    m_dnsServer = QString();
  } else {
    QJsonValue value = json.value("dnsServer");
    if (!value.isString()) {
      logger.error() << "dnsServer is not a string";
      return;
    }
    m_dnsServer = value.toString();
  }

  QJsonArray addrRanges = json.value(JSON_ALLOWED_ADDRESS_RANGES).toArray();
  if (addrRanges.isEmpty()) {
    logger.error() << JSON_ALLOWED_ADDRESS_RANGES << "is empty";
    return;
  }
  for (const QJsonValue& i : addrRanges) {
    // An allowed IP range can be a string of the form: <PREFIX>/<LEN>
    if (i.isString()) {
      m_allowedIPAddressRanges.append(IPAddress(i.toString()));
      continue;
    }

    // Otherwise, it must be an object with separate address and range fields.
    if (!i.isObject()) {
      logger.error() << JSON_ALLOWED_ADDRESS_RANGES
                     << "contains invalid object";
      return;
    }

    QJsonObject ipObj = i.toObject();
    QJsonValue address = ipObj.value("address");
    if (!address.isString()) {
      logger.error() << JSON_ALLOWED_ADDRESS_RANGES
                     << "objects must have a string address";
      return;
    }

    QJsonValue range = ipObj.value("range");
    if (!range.isDouble()) {
      logger.error() << JSON_ALLOWED_ADDRESS_RANGES
                     << "object must have a numberic range";
      return;
    }

    m_allowedIPAddressRanges.append(
        IPAddress(QHostAddress(address.toString()), range.toInt()));
  }

  // Sort allowed IPs by decreasing prefix length.
  std::sort(m_allowedIPAddressRanges.begin(), m_allowedIPAddressRanges.end(),
            [&](const IPAddress& a, const IPAddress& b) -> bool {
              return a.prefixLength() > b.prefixLength();
            });

  m_hopType = getStringEnum(json, "hopType", SingleHop);
  m_vpnDisabledApps = getStringList(json.value("vpnDisabledApps"));
  m_obfuscationMethod = getStringEnum(json, "obfuscationMethod",
                                      Server::ObfuscationMethod::NoObfuscation);

  if (!json.contains("lwoVersion")) {
    m_lwoVersion = 1;
  } else {
    QJsonValue value = json.value("lwoVersion");
    if (!value.isDouble()) {
      logger.error() << "lwoVersion is not a number";
      return;
    }
    int lwoVersion = value.toInt();
    if (lwoVersion < 1 || lwoVersion > 2) {
      logger.error() << "lwoVersion" << lwoVersion << "is not valid";
      return;
    }
    m_lwoVersion = lwoVersion;
  }

  // Success!
  guard.dismiss();
}

bool InterfaceConfig::getRequiredString(const QJsonObject& obj,
                                        const QString& name, QString& output) {
  if (!obj.contains(name)) {
    logger.debug() << name << "missing in the json input";
    return false;
  }

  QJsonValue value = obj.value(name);
  if (value.type() != QJsonValue::String) {
    logger.error() << name << " is not a string";
    return false;
  }
  output = value.toString();
  return true;
}

QStringList InterfaceConfig::getStringList(const QJsonValue& value) {
  QStringList list;
  for (const QJsonValue& i : value.toArray()) {
    if (!i.isString()) {
      continue;
    }
    list.append(i.toString());
  }
  return list;
}

template <typename T>
T InterfaceConfig::getStringEnum(const QJsonObject& obj, const QString& name,
                                 T defaultValue) {
  if (!obj.contains(name)) {
    return defaultValue;
  }
  QJsonValue value = obj.value(name);
  if (!value.isString()) {
    logger.error() << name << " is not a string";
    return defaultValue;
  }

  bool okay;
  QMetaEnum meta = QMetaEnum::fromType<T>();
  T result = T(meta.keyToValue(value.toString().toUtf8().constData(), &okay));
  if (okay) {
    return result;
  }

  logger.error() << name << value.toString() << "is not valid";
  return defaultValue;
}

bool InterfaceConfig::isValid() const {
  return !m_privateKey.isEmpty() && !m_publicKey.isEmpty() &&
         !m_deviceIpv4Address.isEmpty() && !m_deviceIpv6Address.isEmpty();
}

QJsonObject InterfaceConfig::toJson() const {
  QJsonObject json;
  QMetaEnum metaEnum = QMetaEnum::fromType<HopType>();

  json.insert("hopType", QJsonValue(metaEnum.valueToKey(m_hopType)));
  json.insert("privateKey", QJsonValue(m_privateKey));
  json.insert("publicKey", QJsonValue(m_publicKey));
  json.insert("deviceIpv4Address", QJsonValue(m_deviceIpv4Address));
  json.insert("deviceIpv6Address", QJsonValue(m_deviceIpv6Address));
  json.insert("serverPublicKey", QJsonValue(m_serverPublicKey));
  json.insert("serverIpv4AddrIn", QJsonValue(m_serverIpv4AddrIn));
  json.insert("serverIpv6AddrIn", QJsonValue(m_serverIpv6AddrIn));
  json.insert("serverPort", QJsonValue((double)m_serverPort));
  if ((m_hopType == InterfaceConfig::MultiHopExit) ||
      (m_hopType == InterfaceConfig::SingleHop)) {
    json.insert("serverIpv4Gateway", QJsonValue(m_serverIpv4Gateway));
    json.insert("serverIpv6Gateway", QJsonValue(m_serverIpv6Gateway));
    json.insert("dnsServer", QJsonValue(m_dnsServer));
  }

  QJsonArray allowedIPAddesses;
  for (const IPAddress& i : m_allowedIPAddressRanges) {
    QJsonObject range;
    range.insert("address", QJsonValue(i.toHostString()));
    range.insert("range", QJsonValue((double)i.prefixLength()));
    range.insert("isIpv6",
                 QJsonValue(i.protocol() == QAbstractSocket::IPv6Protocol));
    allowedIPAddesses.append(range);
  };
  json.insert("allowedIPAddressRanges", allowedIPAddesses);

  QJsonArray disabledApps;
  for (const QString& i : m_vpnDisabledApps) {
    disabledApps.append(QJsonValue(i));
  }
  json.insert("vpnDisabledApps", disabledApps);

  QMetaEnum obfuscationMetaEnum =
      QMetaEnum::fromType<Server::ObfuscationMethod>();
  json.insert("obfuscationMethod",
              QJsonValue(obfuscationMetaEnum.valueToKey(m_obfuscationMethod)));

  json.insert("lwoVersion", QJsonValue((double)m_lwoVersion));

  return json;
}

QString InterfaceConfig::toWgConf(const QMap<QString, QString>& extra,
                                  const QString peerComment) const {
#define VALIDATE(x) \
  if (x.contains("\n")) return "";
  VALIDATE(m_privateKey);
  VALIDATE(m_deviceIpv4Address);
  VALIDATE(m_deviceIpv6Address);
  VALIDATE(m_serverIpv4Gateway);
  VALIDATE(m_serverIpv6Gateway);
  VALIDATE(m_serverPublicKey);
  VALIDATE(m_serverIpv4AddrIn);
  VALIDATE(m_serverIpv6AddrIn);
#undef VALIDATE

  QString content;
  QTextStream out(&content);

  out << "[Interface]\n";
  out << "PrivateKey = " << m_privateKey << "\n";

  QStringList addresses;
  if (!m_deviceIpv4Address.isNull()) {
    addresses.append(m_deviceIpv4Address);
  }
  if (!m_deviceIpv6Address.isNull()) {
    addresses.append(m_deviceIpv6Address);
  }
  if (addresses.isEmpty()) {
    return "";
  }
  out << "Address = " << addresses.join(", ") << "\n";

  if (!m_dnsServer.isNull()) {
    QStringList dnsServers(m_dnsServer);
    // If the DNS is not the Gateway, it's a user defined DNS
    // thus, not add any other :)
    if (m_dnsServer == m_serverIpv4Gateway) {
      dnsServers.append(m_serverIpv6Gateway);
    }
    out << "DNS = " << dnsServers.join(", ") << "\n";
  }

  // If any extra config was provided, append it now.
  for (const QString& key : extra.keys()) {
    out << key << " = " << extra[key] << "\n";
  }

  out << "\n[Peer]\n";
  if (!peerComment.isEmpty()) {
    for (const QString& line : peerComment.split('\n')) {
      out << "# " << line << "\n";
    }
  }
  out << "PublicKey = " << m_serverPublicKey << "\n";
  if (!m_serverIpv4AddrIn.isEmpty()) {
    out << "Endpoint = " << m_serverIpv4AddrIn << ":" << m_serverPort << "\n";
  } else if (!m_serverIpv6AddrIn.isEmpty()) {
    out << "Endpoint = [" << m_serverIpv6AddrIn << "]:" << m_serverPort << "\n";
  }
  QStringList ranges;
  for (const IPAddress& ip : m_allowedIPAddressRanges) {
    ranges.append(ip.toString());
  }
  out << "AllowedIPs = " << ranges.join(", ") << "\n";

  return content;
}

QString InterfaceConfig::toMultiHopWgConf(
    const InterfaceConfig& exitConfig, const QMap<QString, QString>& extra,
    const QString exitPeerComment, const QString entryPeerComment) const {
  QString content = toWgConf(extra, entryPeerComment);
  QTextStream out(&content);

#define VALIDATE(x) \
  if (x.contains("\n")) return "";
  VALIDATE(exitConfig.m_serverPublicKey);
  VALIDATE(exitConfig.m_serverIpv4AddrIn);
#undef VALIDATE

  out << "\n[Peer]\n";
  if (!exitPeerComment.isEmpty()) {
    for (const QString& line : exitPeerComment.split('\n')) {
      out << "# " << line << "\n";
    }
  }
  out << "PublicKey = " << exitConfig.m_serverPublicKey << "\n";
  if (!exitConfig.m_serverIpv4AddrIn.isNull()) {
    out << "Endpoint = " << exitConfig.m_serverIpv4AddrIn << ":"
        << exitConfig.m_serverPort << "\n";
  } else if (!exitConfig.m_serverIpv6AddrIn.isNull()) {
    out << "Endpoint = [" << exitConfig.m_serverIpv6AddrIn
        << "]:" << exitConfig.m_serverPort << "\n";
  }

  QStringList ranges;
  for (const IPAddress& ip : exitConfig.m_allowedIPAddressRanges) {
    ranges.append(ip.toString());
  }
  out << "AllowedIPs = " << ranges.join(", ") << "\n";

  return content;
}
