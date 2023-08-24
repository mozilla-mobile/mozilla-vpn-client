/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC4193_H
#define RFC4193_H

#include <QList>

#include "ipaddress.h"

class RFC4193 final {
 public:
  static QList<IPAddress> ipv6();
  static bool contains(const QHostAddress& ip);
};

#endif  // RFC4193_H
