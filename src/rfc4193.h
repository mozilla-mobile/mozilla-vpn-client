/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC4193_H
#define RFC4193_H

#include "ipaddressrange.h"

#include <QList>

class RFC4193 final {
 public:
  static QList<IPAddressRange> ipv6();
};

#endif  // RFC4193_H
