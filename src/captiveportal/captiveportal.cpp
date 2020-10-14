/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportal.h"

void CaptivePortal::addIpv4Address(const QString &address)
{
    m_ipv4Addresses.append(address);
}

void CaptivePortal::addIpv6Address(const QString &address)
{
    m_ipv6Addresses.append(address);
}
