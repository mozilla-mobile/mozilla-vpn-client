/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNUSER_H
#define VPNUSER_H

#include <QQmlEngine>

#include "models/user.h"
#include "mozillavpn.h"

struct VPNUser {
  Q_GADGET
  QML_FOREIGN(User)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static User* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->user();
  }
};

#endif  // VPNUSER_H
