/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QHostAddress>
#include <QList>

#include "dnshelper.h"

class IPAddress;
class Server;

/**
 * Controller.cpp is currently mock'ed out of the unit tests
 * stopping us from testing it. This namespace should contain
 * pure functions so that we can start writing unit-tests for that code.
 *
 */
namespace ControllerPrivate {

// The Mullvad proxy services are located at internal IPv4 addresses in the
// 10.124.0.0/20 address range, which is a subset of the 10.0.0.0/8 Class-A
// private address range.
constexpr const char* MULLVAD_PROXY_RANGE = "10.124.0.0";
constexpr const int MULLVAD_PROXY_RANGE_LENGTH = 20;

QList<IPAddress> getExtensionProxyAddressRanges(
    const Server& exitServer,
    std::optional<const dnsData> dnsServer = std::nullopt);

}  // namespace ControllerPrivate
