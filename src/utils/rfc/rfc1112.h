/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC1112_H
#define RFC1112_H

#include "ipaddress.h"

class RFC1112 final {
 public:
  static IPAddress ipv4MulticastAddressBlock();
};

#endif  // RFC1112_H
