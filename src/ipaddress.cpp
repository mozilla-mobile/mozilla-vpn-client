/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"

#include <QtMath>

namespace {
Logger logger(LOG_NETWORKING, "IPAddress");
}

static quint32 s_allOnes = static_cast<quint32>(qPow(2, 32) - 1);

// static
IPAddress IPAddress::create(const QString& ip) {
  if (ip.contains("/")) {
    QPair<QHostAddress, int> p = QHostAddress::parseSubnet(ip);
    Q_ASSERT(p.first.protocol() == QAbstractSocket::IPv4Protocol);

    if (p.second < 32) {
      return IPAddress(p.first, p.second);
    }

    return IPAddress(p.first);
  }

  return IPAddress(QHostAddress(ip));
}

IPAddress::IPAddress() { MVPN_COUNT_CTOR(IPAddress); }

IPAddress::IPAddress(const IPAddress& other) {
  MVPN_COUNT_CTOR(IPAddress);
  *this = other;
}

IPAddress& IPAddress::operator=(const IPAddress& other) {
  if (this == &other) return *this;

  m_address = other.m_address;
  m_prefixLength = other.m_prefixLength;
  m_netmask = other.m_netmask;
  m_hostmask = other.m_hostmask;
  m_broadcastAddress = other.m_broadcastAddress;

  return *this;
}

IPAddress::IPAddress(const QHostAddress& address)
    : m_address(address),
      m_prefixLength(32),
      m_netmask(QHostAddress(s_allOnes)),
      m_hostmask(QHostAddress((quint32)(0))),
      m_broadcastAddress(address) {
  MVPN_COUNT_CTOR(IPAddress);
  Q_ASSERT(address.protocol() == QAbstractSocket::IPv4Protocol);
}

IPAddress::IPAddress(const QHostAddress& address, int prefixLength)
    : m_address(address), m_prefixLength(prefixLength) {
  MVPN_COUNT_CTOR(IPAddress);
  Q_ASSERT(address.protocol() == QAbstractSocket::IPv4Protocol);

  Q_ASSERT(prefixLength >= 0 && prefixLength <= 32);
  m_netmask = QHostAddress(s_allOnes ^ (s_allOnes >> prefixLength));

  m_hostmask = QHostAddress(m_netmask.toIPv4Address() ^ s_allOnes);
  m_broadcastAddress =
      QHostAddress(address.toIPv4Address() | m_hostmask.toIPv4Address());
}

IPAddress::~IPAddress() { MVPN_COUNT_DTOR(IPAddress); }

bool IPAddress::overlaps(const IPAddress& other) const {
  return other.contains(m_address) || other.contains(m_broadcastAddress) ||
         contains(other.m_address) || contains(other.m_broadcastAddress);
}

bool IPAddress::contains(const QHostAddress& address) const {
  return (m_address.toIPv4Address() <= address.toIPv4Address()) &&
         (address.toIPv4Address() <= m_broadcastAddress.toIPv4Address());
}

bool IPAddress::operator==(const IPAddress& other) const {
  return m_address == other.m_address && m_netmask == other.m_netmask;
}

bool IPAddress::subnetOf(const IPAddress& other) const {
  return other.m_address.toIPv4Address() <= m_address.toIPv4Address() &&
         other.m_broadcastAddress.toIPv4Address() >=
             m_broadcastAddress.toIPv4Address();
}

QList<IPAddress> IPAddress::subnets() const {
  quint64 start = m_address.toIPv4Address();
  quint64 end = quint64(m_broadcastAddress.toIPv4Address()) + 1;
  quint64 step = ((quint64)m_hostmask.toIPv4Address() + 1) >> 1;

  QList<IPAddress> list;

  if (m_prefixLength == 32) {
    list.append(*this);
    return list;
  }

  while (start < end) {
    int newPrefixLength = m_prefixLength + 1;
    if (newPrefixLength == 32) {
      list.append(IPAddress(QHostAddress(start)));
    } else {
      list.append(IPAddress(QHostAddress(start), m_prefixLength + 1));
    }
    start += step;
  }

  return list;
}

// static
QList<IPAddress> IPAddress::excludeAddresses(
    const QList<IPAddress>& sourceList, const QList<IPAddress>& excludeList) {
  QList<IPAddress> results = sourceList;

  for (const IPAddress& exclude : excludeList) {
    QList<IPAddress> newResults;

    for (const IPAddress& ip : results) {
      if (ip.overlaps(exclude)) {
        QList<IPAddress> range = ip.excludeAddresses(exclude);
        newResults.append(range);
      } else {
        newResults.append(ip);
      }
    }

    results = newResults;
  }

  return results;
}

QList<IPAddress> IPAddress::excludeAddresses(const IPAddress& ip) const {
  QList<IPAddress> sn = subnets();
  Q_ASSERT(sn.length() >= 2);

  QList<IPAddress> result;
  while (sn[0] != ip && sn[1] != ip) {
    if (ip.subnetOf(sn[0])) {
      result.append(sn[1]);
      sn = sn[0].subnets();
    } else if (ip.subnetOf(sn[1])) {
      result.append(sn[0]);
      sn = sn[1].subnets();
    } else {
      Q_ASSERT(false);
    }
  }

  if (sn[0] == ip) {
    result.append(sn[1]);
  } else if (sn[1] == ip) {
    result.append(sn[0]);
  } else {
    Q_ASSERT(false);
  }

  return result;
}
