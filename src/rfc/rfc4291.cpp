/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rfc4291.h"

// static
IPAddress RFC4291::ipv6LoopbackAddressBlock() { return IPAddress("::1/128"); }

// static
IPAddress RFC4291::ipv6MulticastAddressBlock() { return IPAddress("ff00::/8"); }
