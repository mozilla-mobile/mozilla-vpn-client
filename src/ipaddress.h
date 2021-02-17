/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IPADDRESS_H
#define IPADDRESS_H

#include <QHostAddress>

class IPAddress final {
 public:
  static IPAddress create(const QString& ip);
  static QList<IPAddress> excludeAddresses(const QList<IPAddress>& sourceList,
                                           const QList<IPAddress>& excludeList);

  IPAddress();
  IPAddress(const IPAddress& other);
  IPAddress& operator=(const IPAddress& other);
  ~IPAddress();

  QString toString() const {
    return QString("%1/%2").arg(m_address.toString()).arg(m_prefixLength);
  }

  const QHostAddress& address() const { return m_address; }
  int prefixLength() const { return m_prefixLength; }
  const QHostAddress& netmask() const { return m_netmask; }
  const QHostAddress& hostmask() const { return m_hostmask; }
  const QHostAddress& broadcastAddress() const { return m_broadcastAddress; }

  bool overlaps(const IPAddress& other) const;

  bool contains(const QHostAddress& address) const;

  bool operator==(const IPAddress& other) const;
  bool operator!=(const IPAddress& other) const { return !operator==(other); }

  bool subnetOf(const IPAddress& other) const;

  QList<IPAddress> subnets() const;

  QList<IPAddress> excludeAddresses(const IPAddress& ip) const;

 private:
  IPAddress(const QHostAddress& address);
  IPAddress(const QHostAddress& address, int prefixLength);

 private:
  QHostAddress m_address;
  int m_prefixLength;

  QHostAddress m_netmask;
  QHostAddress m_hostmask;
  QHostAddress m_broadcastAddress;
};

#endif  // IPADDRESS_H
