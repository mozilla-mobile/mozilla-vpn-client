/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RFC1918_H
#define RFC1918_H

#include <QHostAddress>
#include <QList>

#include "ipaddress.h"

class RFC1918 final {
 public:
  static QList<IPAddress> ipv4();
  static bool contains(const QHostAddress& ip);
};

#endif  // RFC1918_H
