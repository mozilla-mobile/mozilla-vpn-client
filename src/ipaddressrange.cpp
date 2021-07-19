/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ipaddressrange.h"
#include "ipaddress.h"
#include "leakdetector.h"

IPAddressRange::IPAddressRange(const QString& ipAddress, uint32_t range,
                               IPAddressType type)
    : m_ipAddress(ipAddress), m_range(range), m_type(type) {
  MVPN_COUNT_CTOR(IPAddressRange);
}

IPAddressRange::IPAddressRange(const QString& prefix) {
  MVPN_COUNT_CTOR(IPAddressRange);
  QStringList split = prefix.split('/');
  m_ipAddress = split[0];
  if (m_ipAddress.contains(':')) {
    // Probably IPv6
    m_type = IPv6;
    m_range = 128;
  } else {
    // Assume IPv4
    m_type = IPv4;
    m_range = 32;
  }
  if (split.count() > 1) {
    m_range = split[1].toUInt();
  }
}

IPAddressRange::IPAddressRange(const IPAddressRange& other) {
  MVPN_COUNT_CTOR(IPAddressRange);
  *this = other;
}

IPAddressRange& IPAddressRange::operator=(const IPAddressRange& other) {
  if (this == &other) return *this;

  m_ipAddress = other.m_ipAddress;
  m_range = other.m_range;
  m_type = other.m_type;

  return *this;
}

IPAddressRange::~IPAddressRange() { MVPN_COUNT_DTOR(IPAddressRange); }

// static
QList<IPAddressRange> IPAddressRange::fromIPAddressList(
    const QList<IPAddress>& list) {
  QList<IPAddressRange> result;
  for (const IPAddress& ip : list) {
    result.append(IPAddressRange(ip.toString()));
  }
  return result;
}
