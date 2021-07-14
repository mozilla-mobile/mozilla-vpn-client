/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC4291_H
#define RFC4291_H

#include "ipaddressrange.h"

#include <QList>

class RFC4291 final {
 public:
  static QList<IPAddressRange> ipv6unicast();
};

#endif  // RFC4291_H
