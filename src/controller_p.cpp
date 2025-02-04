/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller_p.h"

#include <qassert.h>
#include <qhostaddress.h>

#include "dnshelper.h"
#include "ipaddress.h"
#include "models/server.h"
#include "rfc/rfc1112.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"

namespace ControllerPrivate {

QList<IPAddress> getExtensionProxyAddressRanges(
    const Server& exitServer, std::optional<const dnsData> dnsServer) {
  QList<IPAddress> ranges = {
      IPAddress(QHostAddress(exitServer.ipv4Gateway()), 32),
      IPAddress(QHostAddress{MULLVAD_PROXY_RANGE}, MULLVAD_PROXY_RANGE_LENGTH)};
  if (!exitServer.ipv6Gateway().isEmpty()) {
    ranges.append(IPAddress(QHostAddress(exitServer.ipv6Gateway()), 128));
  }

  const dnsData dns = [&dnsServer, &exitServer]() {
    if (dnsServer.has_value()) {
      return dnsServer.value();
    }
    return DNSHelper::getDNSDetails(exitServer.ipv4Gateway());
  }();

  if (dns.dnsType == "Default") {
    // The DNS Adress is already Resolved.
    return ranges;
  }
  if (dns.dnsType != "Custom") {
    // It's a vpn-internal DNS, always add it
    ranges.append(IPAddress(QHostAddress(dns.ipAddress), 32));
    return ranges;
  }
  Q_ASSERT(dns.dnsType == "Custom");
  const QHostAddress addr{dns.ipAddress};
  // If it is a custom dns only add it to the tunnel if it is **NOT**
  // a "lan" adress
  if (!RFC1918::contains(addr) && !RFC4193::contains(addr) &&
      !addr.isLoopback()) {
    ranges.append(IPAddress(QHostAddress(dns.ipAddress), 32));
  }
  return ranges;
}

}  // namespace ControllerPrivate
