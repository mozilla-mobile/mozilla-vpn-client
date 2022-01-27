/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc5735.h"

// static
IPAddress RFC5735::ipv4LoopbackAddressBlock() {
  // https://datatracker.ietf.org/doc/html/rfc5735#section-3
  return IPAddress("127.0.0.0/8");
}
