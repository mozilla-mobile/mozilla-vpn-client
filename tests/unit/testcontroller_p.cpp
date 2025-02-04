/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testcontroller_p.h"

#include <qhostaddress.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qtestcase.h>
#include <qtpreprocessorsupport.h>

#include "controller_p.h"
#include "dnshelper.h"
#include "ipaddress.h"
#include "models/server.h"
namespace {

Server createServer(QString ipv4Gateway) {
  QJsonObject args;
  args["hostname"] = "test";
  args["ipv4_addr_in"] = "test";
  args["ipv4_gateway"] = ipv4Gateway;
  args["ipv6_addr_in"] = "2001:db8::1";
  args["ipv6_gateway"] = "2001:db8::1";
  args["public_key"] = "test";
  args["weight"] = 0.0;
  args["port_ranges"] = QJsonArray{};
  Server out{};
  bool ok = out.fromJson(args);
  Q_UNUSED(ok);
  return out;
}

bool includesAddress(QHostAddress addr, const QList<IPAddress>& l) {
  for (auto const& laddr : l) {
    if (laddr.contains(addr)) {
      return true;
    }
  }
  return false;
}

};  // namespace

void TestControllerPrivate::getExtensionProxyAddressRanges() {
  auto const testIsDNSIncluded = [](dnsData dns, QString exitGateway) {
    auto const res = ControllerPrivate::getExtensionProxyAddressRanges(
        createServer(exitGateway), dns);
    return includesAddress(QHostAddress{dns.ipAddress}, res);
  };

  // A Custom DNS that is not in the localhost network
  // should be in the ProxyAdressRange (so it will be tunneled in the vpn)
  QCOMPARE(testIsDNSIncluded(
               {
                   .ipAddress = "99.99.99.99",
                   .dnsType = "Custom",
               },
               "1.2.3.5"),
           true);

  // Opposite: a LAN ip should not appear on this list.
  QCOMPARE(testIsDNSIncluded(
               {
                   .ipAddress = "192.168.0.1",
                   .dnsType = "Custom",
               },
               "1.2.3.5"),
           false);

  // If you run your dns on localhost. also not.
  QCOMPARE(testIsDNSIncluded(
               {
                   .ipAddress = "127.0.0.1",
                   .dnsType = "Custom",
               },
               "1.2.3.5"),
           false);

  // The default is the Exit node, so that should be tunneled
  QCOMPARE(testIsDNSIncluded(
               {
                   .ipAddress = "1.2.3.5",
                   .dnsType = "Default",
               },
               "1.2.3.5"),
           true);

  // If we add a filtering dns, that MUST always go through the tunnel.
  QCOMPARE(testIsDNSIncluded(
               {
                   .ipAddress = "99.99.99.99",
                   .dnsType = "BlockMalwareAndTrackers",
               },
               "1.2.3.5"),
           true);
}

static TestControllerPrivate s_instance;
