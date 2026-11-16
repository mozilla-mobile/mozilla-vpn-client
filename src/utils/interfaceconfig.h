/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INTERFACECONFIG_H
#define INTERFACECONFIG_H

#include <QList>
#include <QMap>
#include <QString>

#include "ipaddress.h"
#include "models/server.h"

class QJsonObject;

class InterfaceConfig {
  Q_GADGET

 public:
  InterfaceConfig() {}
  InterfaceConfig(const QJsonObject&);

  // Device configuration
  QString m_privateKey;
  QString m_publicKey;
  QString m_deviceIpv4Address;
  QString m_deviceIpv6Address;

  // Wireguard server configuration.
  QString m_serverPublicKey;
  QString m_serverCityName;
  QString m_serverIpv4Gateway;
  QString m_serverIpv6Gateway;
  QString m_serverIpv4AddrIn;
  QString m_serverIpv6AddrIn;
  int m_serverPort = 0;

  // Multihop entry server configuration.
  QString m_multihopPublicKey;
  QString m_multihopCityName;
  QString m_multihopIpv4AddrIn;
  QString m_multihopIpv6AddrIn;
  int m_multihopPort = 0;

  // Misc configuration
  QString m_dnsServer;
  QList<IPAddress> m_allowedIPAddressRanges;
  QStringList m_vpnDisabledApps;
  Server::ObfuscationMethod m_obfuscationMethod = Server::NoObfuscation;
  int m_lwoVersion = 1;

  bool isValid() const;

  QJsonObject toJson() const;
  QString toWgConf(
      const QMap<QString, QString>& extra = QMap<QString, QString>(),
      const QString peerComment = "") const;

 private:
  // Helper methods for parsing JSON
  static bool getRequiredString(const QJsonObject& obj, const QString& name,
                                QString& out);
  static QStringList getStringList(const QJsonValue& obj);

  template <typename T>
  static T getStringEnum(const QJsonObject& obj, const QString& name,
                         T defaultValue = 0);
};

#endif  // INTERFACECONFIG_H
