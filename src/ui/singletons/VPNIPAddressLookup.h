/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNIPADDRESSLOOKUP_H
#define VPNIPADDRESSLOOKUP_H

#include <QQmlEngine>

#include "ipaddresslookup.h"
#include "mozillavpn.h"

struct VPNIPAddressLookup {
  Q_GADGET
  QML_FOREIGN(IpAddressLookup)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static IpAddressLookup* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->ipAddressLookup();
  }
};

#endif  // VPNIPADDRESSLOOKUP_H
