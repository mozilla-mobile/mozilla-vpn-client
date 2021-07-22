/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc4193.h"

// static
QList<IPAddressRange> RFC4193::ipv6() {
  // Allow all IP's except fc00::/7
  QList<IPAddressRange> list;
  list.append(IPAddressRange("0:0:0:0:0:0:0:0", 1, IPAddressRange::IPv6));
  list.append(IPAddressRange("8000::", 2, IPAddressRange::IPv6));
  list.append(IPAddressRange("c000::", 3, IPAddressRange::IPv6));
  list.append(IPAddressRange("e000::", 4, IPAddressRange::IPv6));
  list.append(IPAddressRange("f000::", 5, IPAddressRange::IPv6));
  list.append(IPAddressRange("f800::", 6, IPAddressRange::IPv6));

  list.append(IPAddressRange("fc01::", 16, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc02::", 15, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc04::", 14, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc08::", 13, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc10::", 12, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc20::", 11, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc40::", 10, IPAddressRange::IPv6));
  list.append(IPAddressRange("fc80::", 9, IPAddressRange::IPv6));
  list.append(IPAddressRange("fd00::", 8, IPAddressRange::IPv6));
  list.append(IPAddressRange("fe00::", 7, IPAddressRange::IPv6));
  return list;
}
