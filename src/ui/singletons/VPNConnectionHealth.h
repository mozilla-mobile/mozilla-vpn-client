/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCONNECTIONHEALTH_H
#define VPNCONNECTIONHEALTH_H

#include <QQmlEngine>

#include "connectionhealth.h"
#include "mozillavpn.h"

struct VPNConnectionHealth {
  Q_GADGET
  QML_FOREIGN(ConnectionHealth)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ConnectionHealth* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->connectionHealth();
  }
};

#endif  // VPNCONNECTIONHEALTH_H
