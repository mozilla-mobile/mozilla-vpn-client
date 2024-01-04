/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNSERVERLATENCY_H
#define VPNSERVERLATENCY_H

#include <QQmlEngine>

#include "mozillavpn.h"
#include "serverlatency.h"

struct VPNServerLatency {
  Q_GADGET
  QML_FOREIGN(ServerLatency)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ServerLatency* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->serverLatency();
  }
};

#endif  // VPNSERVERLATENCY_H
