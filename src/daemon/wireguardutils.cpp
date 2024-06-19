/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutils.h"

#include "ipaddress.h"

bool WireguardUtils::excludeLocalNetworks(const QList<IPAddress>& addresses) {
  // Determine the IP address prefixes which match everything except the LAN.
  QList<IPAddress> defaultRoutes;
  defaultRoutes.append(IPAddress("0.0.0.0/0"));
  defaultRoutes.append(IPAddress("::/0"));

  // Route them into the VPN.
  bool result = true;
  for (auto prefix : IPAddress::excludeAddresses(defaultRoutes, addresses)) {
    if (!updateRoutePrefix(prefix)) {
        result = false;
    }
  }

  return result;
}
