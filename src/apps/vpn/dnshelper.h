/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DNSHELPER_H
#define DNSHELPER_H

#include <QString>

class DNSHelper final {
 public:
  static QString getDNS(const QString& fallbackAddress);

  static QString getDNSType();

  static bool validateUserDNS(const QString& dns);

  static void maybeMigrateDNSProviderFlags();
};

#endif  // DNSHELPER_H
