/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc1918.h"

// static
QList<IPAddress> RFC1918::ipv4() {
  QList<IPAddress> list;

  // From RFC1918: https://tools.ietf.org/html/rfc1918
  list.append(IPAddress::create("10.0.0.0/8"));
  list.append(IPAddress::create("172.16.0.0/12"));
  list.append(IPAddress::create("192.168.0.0/16"));

  return list;
}

bool RFC1918::contains(const QHostAddress& ip) {
  QList<IPAddress> list = ipv4();
  foreach (const IPAddress& addr, list) {
    if (addr.contains(ip)) {
      return true;
    }
  }
  return false;
}
