/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc4193.h"

// static
QList<IPAddress> RFC4193::ipv6() {
  QList<IPAddress> list;

  list.append(IPAddress::create("fc00::/7"));

  return list;
}

bool RFC4193::contains(const QHostAddress& ip) {
  QList<IPAddress> list = ipv6();
  foreach (const IPAddress& addr, list) {
    if (addr.contains(ip)) {
      return true;
    }
  }
  return false;
}
