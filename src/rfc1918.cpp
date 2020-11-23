/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc1918.h"

// static
QList<IPAddressRange> RFC1918::ipv4() {
  QList<IPAddressRange> list;

  // From RFC1918: https://tools.ietf.org/html/rfc1918
  list.append(IPAddressRange("0.0.0.0", 5, IPAddressRange::IPv4));
  list.append(IPAddressRange("8.0.0.0", 7, IPAddressRange::IPv4));
  list.append(IPAddressRange("11.0.0.0", 8, IPAddressRange::IPv4));
  list.append(IPAddressRange("12.0.0.0", 6, IPAddressRange::IPv4));
  list.append(IPAddressRange("16.0.0.0", 4, IPAddressRange::IPv4));
  list.append(IPAddressRange("32.0.0.0", 3, IPAddressRange::IPv4));
  list.append(IPAddressRange("64.0.0.0", 2, IPAddressRange::IPv4));
  list.append(IPAddressRange("128.0.0.0", 3, IPAddressRange::IPv4));
  list.append(IPAddressRange("160.0.0.0", 5, IPAddressRange::IPv4));
  list.append(IPAddressRange("168.0.0.0", 6, IPAddressRange::IPv4));
  list.append(IPAddressRange("172.0.0.0", 12, IPAddressRange::IPv4));
  list.append(IPAddressRange("172.32.0.0", 11, IPAddressRange::IPv4));
  list.append(IPAddressRange("172.64.0.0", 10, IPAddressRange::IPv4));
  list.append(IPAddressRange("172.128.0.0", 9, IPAddressRange::IPv4));
  list.append(IPAddressRange("173.0.0.0", 8, IPAddressRange::IPv4));
  list.append(IPAddressRange("174.0.0.0", 7, IPAddressRange::IPv4));
  list.append(IPAddressRange("176.0.0.0", 4, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.0.0.0", 9, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.128.0.0", 11, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.160.0.0", 13, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.169.0.0", 16, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.170.0.0", 15, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.172.0.0", 14, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.176.0.0", 12, IPAddressRange::IPv4));
  list.append(IPAddressRange("192.192.0.0", 10, IPAddressRange::IPv4));
  list.append(IPAddressRange("193.0.0.0", 8, IPAddressRange::IPv4));
  list.append(IPAddressRange("194.0.0.0", 7, IPAddressRange::IPv4));
  list.append(IPAddressRange("196.0.0.0", 6, IPAddressRange::IPv4));
  list.append(IPAddressRange("200.0.0.0", 5, IPAddressRange::IPv4));
  list.append(IPAddressRange("208.0.0.0", 4, IPAddressRange::IPv4));

  return list;
}

// static
QList<IPAddressRange> RFC1918::ipv6() { return QList<IPAddressRange>(); }
