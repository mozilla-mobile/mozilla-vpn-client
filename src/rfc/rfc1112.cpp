/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc1112.h"

// static
IPAddress RFC1112::ipv4MulticastAddressBlock() {
  return IPAddress("224.0.0.0/4");
}
