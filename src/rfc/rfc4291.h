/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC4291_H
#define RFC4291_H

#include "ipaddress.h"

// Clearly, this is not the full implementation of the RFC4291. We care just
// about the loopback and multicast blocks.
class RFC4291 final {
 public:
  static IPAddress ipv6LoopbackAddressBlock();
  static IPAddress ipv6MulticastAddressBlock();
};

#endif  // RFC4291_H
