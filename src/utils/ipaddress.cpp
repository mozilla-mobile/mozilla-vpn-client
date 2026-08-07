/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ipaddress.h"

#include <QtMath>

#include "leakdetector.h"
#include "rfc/rfc1112.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"

IPAddress::IPAddress() { MZ_COUNT_CTOR(IPAddress); }

IPAddress::IPAddress(const QString& ip) : QHostAddress() {
  MZ_COUNT_CTOR(IPAddress);
  if (ip.contains("/")) {
    QPair<QHostAddress, int> p = QHostAddress::parseSubnet(ip);
    *reinterpret_cast<QHostAddress*>(this) = p.first;
    m_prefixLength = p.second;
  } else {
    setAddress(ip);
    m_prefixLength = 999999;
  }

  if (protocol() == QAbstractSocket::IPv4Protocol) {
    if (m_prefixLength >= 32) {
      m_prefixLength = 32;
    }
  } else if (protocol() == QAbstractSocket::IPv6Protocol) {
    if (m_prefixLength >= 128) {
      m_prefixLength = 128;
    }
  }
}

IPAddress::IPAddress(const IPAddress& other) {
  MZ_COUNT_CTOR(IPAddress);
  *this = other;
}

IPAddress& IPAddress::operator=(const IPAddress& other) {
  if (this == &other) return *this;

  *reinterpret_cast<QHostAddress*>(this) = other;
  m_prefixLength = other.m_prefixLength;

  return *this;
}

IPAddress::IPAddress(const QHostAddress& address) : QHostAddress(address) {
  MZ_COUNT_CTOR(IPAddress);

  if (address.protocol() == QAbstractSocket::IPv4Protocol) {
    m_prefixLength = 32;
  } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    m_prefixLength = 128;
  } else if (address.isNull()) {
    m_prefixLength = -1;
  }
}

IPAddress::IPAddress(const QHostAddress& address, int prefixLength)
    : QHostAddress(address), m_prefixLength(prefixLength) {
  MZ_COUNT_CTOR(IPAddress);

  if (m_prefixLength < 0) {
    clear();
  } else if (address.protocol() == QAbstractSocket::IPv4Protocol) {
    if (m_prefixLength > 32) {
      m_prefixLength = -1;
      clear();
    }
  } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    if (m_prefixLength > 128) {
      m_prefixLength = -1;
      clear();
    }
  } else if (address.isNull()) {
    m_prefixLength = -1;
  }
}

IPAddress::~IPAddress() { MZ_COUNT_DTOR(IPAddress); }

QString IPAddress::toString() const {
  return QString("%1/%2").arg(toHostString()).arg(m_prefixLength);
}

QHostAddress IPAddress::netmask() const {
  if ((protocol() == QAbstractSocket::IPv6Protocol) &&
      (m_prefixLength <= 128)) {
    Q_IPV6ADDR rawNetmask = {0};
    memset(&rawNetmask, 0xff, m_prefixLength / 8);
    if (m_prefixLength % 8) {
      rawNetmask[m_prefixLength / 8] = 0xFF ^ (0xFF >> (m_prefixLength % 8));
    }
    return QHostAddress(rawNetmask);
  }

  if ((protocol() == QAbstractSocket::IPv4Protocol) && (m_prefixLength <= 32)) {
    quint32 rawNetmask = 0xffffffff;
    if (m_prefixLength < 32) {
      rawNetmask ^= (0xffffffff >> m_prefixLength);
    }
    return QHostAddress(rawNetmask);
  }

  return QHostAddress();
}

QHostAddress IPAddress::hostmask() const {
  if ((protocol() == QAbstractSocket::IPv6Protocol) &&
      (m_prefixLength <= 128)) {
    Q_IPV6ADDR rawHostmask = {0};
    int offset = (m_prefixLength + 7) / 8;
    memset(&rawHostmask[offset], 0xff, sizeof(rawHostmask) - offset);
    if (m_prefixLength % 8) {
      rawHostmask[m_prefixLength / 8] = 0xFF >> (m_prefixLength % 8);
    }
    return QHostAddress(rawHostmask);
  }

  if (protocol() == QAbstractSocket::IPv4Protocol) {
    if (m_prefixLength < 32) {
      return QHostAddress(0xffffffff >> m_prefixLength);
    } else {
      quint32 zero = 0;
      return QHostAddress(zero);
    }
  }

  return QHostAddress();
}

QHostAddress IPAddress::broadcastAddress() const {
  if (protocol() == QAbstractSocket::IPv6Protocol) {
    Q_IPV6ADDR rawAddress = toIPv6Address();
    if (m_prefixLength >= 128) {
      return QHostAddress(rawAddress);
    }

    int offset = (m_prefixLength + 7) / 8;
    memset(&rawAddress[offset], 0xff, sizeof(rawAddress) - offset);
    if (m_prefixLength % 8) {
      rawAddress[m_prefixLength / 8] |= 0xFF >> (m_prefixLength % 8);
    }
    return QHostAddress(rawAddress);
  }

  if (protocol() == QAbstractSocket::IPv4Protocol) {
    quint32 rawAddress = toIPv4Address();
    if (m_prefixLength < 32) {
      rawAddress |= (0xffffffff >> m_prefixLength);
    }
    return QHostAddress(rawAddress);
  }

  return QHostAddress();
}

bool IPAddress::overlaps(const IPAddress& other) const {
  if (m_prefixLength < other.m_prefixLength) {
    return contains(other);
  } else {
    return other.contains(*this);
  }
}

bool IPAddress::contains(const QHostAddress& address) const {
  if (address.protocol() != protocol()) {
    return false;
  }
  if (m_prefixLength == 0) {
    return true;
  }

  if (protocol() == QAbstractSocket::IPv6Protocol) {
    Q_IPV6ADDR a = toIPv6Address();
    Q_IPV6ADDR b = address.toIPv6Address();
    int bytes = m_prefixLength / 8;
    if (bytes > 0) {
      if (memcmp(&a, &b, bytes) != 0) {
        return false;
      }
    }

    if (m_prefixLength % 8) {
      quint8 diff = (a[bytes] ^ b[bytes]) >> (8 - m_prefixLength % 8);
      return (diff == 0);
    }

    return true;
  }

  if (protocol() == QAbstractSocket::IPv4Protocol) {
    quint32 diff = toIPv4Address() ^ address.toIPv4Address();
    if (m_prefixLength < 32) {
      diff >>= (32 - m_prefixLength);
    }
    return (diff == 0);
  }

  return false;
}

bool IPAddress::operator==(const IPAddress& other) const {
  const QHostAddress* hostpart = reinterpret_cast<const QHostAddress*>(this);
  return hostpart->isEqual(other) && m_prefixLength == other.m_prefixLength;
}

bool IPAddress::subnetOf(const IPAddress& other) const {
  if (other.protocol() != this->protocol()) {
    return false;
  }
  if (m_prefixLength < other.m_prefixLength) {
    return false;
  }

  return other.contains(*this);
}

QList<IPAddress> IPAddress::subnets() const {
  QList<IPAddress> list;

  if (protocol() == QAbstractSocket::IPv4Protocol) {
    if (m_prefixLength >= 32) {
      list.append(*this);
      return list;
    }

    quint32 rawAddress = toIPv4Address();
    list.append(IPAddress(QHostAddress(rawAddress), m_prefixLength + 1));

    rawAddress |= (0x80000000 >> m_prefixLength);
    list.append(IPAddress(QHostAddress(rawAddress), m_prefixLength + 1));
  } else if (protocol() == QAbstractSocket::IPv6Protocol) {
    if (m_prefixLength >= 128) {
      list.append(*this);
      return list;
    }

    Q_IPV6ADDR rawAddress = toIPv6Address();
    list.append(IPAddress(QHostAddress(rawAddress), m_prefixLength + 1));

    rawAddress[m_prefixLength / 8] |= (0x80 >> (m_prefixLength % 8));
    list.append(IPAddress(QHostAddress(rawAddress), m_prefixLength + 1));
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
      if (!ip.overlaps(exclude)) {
        newResults.append(ip);
      } else if (exclude.subnetOf(ip) && exclude != ip) {
        QList<IPAddress> range = ip.excludeAddresses(exclude);
        newResults.append(range);
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

// static
QList<IPAddress> IPAddress::lanAddressRanges() {
  QList<IPAddress> ranges;

  // filtering out the RFC1918 local area network
  ranges.append(RFC1918::ipv4());

  ranges.append(RFC4193::ipv6());
  ranges.append(RFC4291::ipv6LinkLocalAddressBlock());

  ranges.append(RFC1112::ipv4MulticastAddressBlock());
  ranges.append(RFC4291::ipv6MulticastAddressBlock());

  return ranges;
}
