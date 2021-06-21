/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC5735_H
#define RFC5735_H

#include "ipaddress.h"

// Clearly, this is not the full implementation of the RFC5735. We care just
// about the loopback block.
class RFC5735 final {
 public:
  static IPAddress ipv4LoopbackAddressBlock();
};

#endif  // RFC5735_H
