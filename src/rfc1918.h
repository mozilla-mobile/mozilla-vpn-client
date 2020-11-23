/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC1918_H
#define RFC1918_H

#include "ipaddressrange.h"

#include <QList>

class RFC1918 final {
 public:
  static QList<IPAddressRange> ipv4();
  static QList<IPAddressRange> ipv6();
};

#endif  // RFC1918_H
