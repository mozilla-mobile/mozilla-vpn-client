/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC4193_H
#define RFC4193_H

#include "ipaddressrange.h"

#include <QList>

class RFC4193 final {
 public:
  // Note: this returns the "opposite" of the RFC4193: what does not be treated
  // as local network.
  static QList<IPAddressRange> ipv6();
};

#endif  // RFC4193_H
