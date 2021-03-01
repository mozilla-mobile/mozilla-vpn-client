/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILS_H
#define WIREGUARDUTILS_H

#include "ipaddressrange.h"

#include <QCoreApplication>

constexpr const char* WG_INTERFACE = "moz0";

struct InterfaceConfig {
  QString m_privateKey;
  QString m_deviceIpv4Address;
  QString m_deviceIpv6Address;
  QString m_serverIpv4Gateway;
  QString m_serverIpv6Gateway;
  QString m_serverPublicKey;
  QString m_serverIpv4AddrIn;
  QString m_serverIpv6AddrIn;
  int m_serverPort = 0;
  bool m_ipv6Enabled = false;
  QList<IPAddressRange> m_allowedIPAddressRanges;
};

class WireguardUtils : public QObject {
 public:
  virtual bool interfaceExists() {
    qFatal("Have you forgotten to implement WireguardUtils::interfaceExists?");
    return false;
  };

  virtual QStringList currentInterfaces() {
    qFatal(
        "Have you forgotten to implement WireguardUtils::currentInterfaces?");
    return QStringList();
  };
};

#endif  // WIREGUARDUTILS_H
