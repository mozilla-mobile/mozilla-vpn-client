/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNPROFILEFLOW_H
#define VPNPROFILEFLOW_H

#include <QQmlEngine>

#include "mozillavpn.h"
#include "profileflow.h"

struct VPNProfileFlow {
  Q_GADGET
  QML_FOREIGN(ProfileFlow)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ProfileFlow* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->profileFlow();
  }
};

#endif  // VPNPROFILEFLOW_H
