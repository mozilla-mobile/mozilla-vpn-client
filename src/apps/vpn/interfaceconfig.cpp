/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "interfaceconfig.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

QJsonObject InterfaceConfig::toJson() const {
  QJsonObject json;
  json.insert("hopindex", QJsonValue((double)m_hopindex));
  json.insert("privateKey", QJsonValue(m_privateKey));
  json.insert("deviceIpv4Address", QJsonValue(m_deviceIpv4Address));
  json.insert("deviceIpv6Address", QJsonValue(m_deviceIpv6Address));
  json.insert("serverPublicKey", QJsonValue(m_serverPublicKey));
  json.insert("serverIpv4AddrIn", QJsonValue(m_serverIpv4AddrIn));
  json.insert("serverIpv6AddrIn", QJsonValue(m_serverIpv6AddrIn));
  json.insert("serverPort", QJsonValue((double)m_serverPort));
  if (m_hopindex == 0) {
    json.insert("serverIpv4Gateway", QJsonValue(m_serverIpv4Gateway));
    json.insert("serverIpv6Gateway", QJsonValue(m_serverIpv6Gateway));
    json.insert("dnsServer", QJsonValue(m_dnsServer));
  }

  QJsonArray allowedIPAddesses;
  for (const IPAddress& i : m_allowedIPAddressRanges) {
    QJsonObject range;
    range.insert("address", QJsonValue(i.address().toString()));
    range.insert("range", QJsonValue((double)i.prefixLength()));
    range.insert("isIpv6",
                 QJsonValue(i.type() == QAbstractSocket::IPv6Protocol));
    allowedIPAddesses.append(range);
  };
  json.insert("allowedIPAddressRanges", allowedIPAddesses);

  QJsonArray jsExcludedAddresses;
  for (const QString& i : m_excludedAddresses) {
    jsExcludedAddresses.append(QJsonValue(i));
  }
  json.insert("excludedAddresses", jsExcludedAddresses);

  QJsonArray disabledApps;
  for (const QString& i : m_vpnDisabledApps) {
    disabledApps.append(QJsonValue(i));
  }
  json.insert("vpnDisabledApps", disabledApps);

  return json;
}
