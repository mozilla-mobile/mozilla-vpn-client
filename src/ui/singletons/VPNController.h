/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCONTROLLER_H
#define VPNCONTROLLER_H

#include <QQmlEngine>

#include "controller.h"
#include "mozillavpn.h"

class Controller;

struct VPNController {
  Q_GADGET
  QML_FOREIGN(Controller)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static Controller* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->controller();
  }
};

#endif  // VPNCONTROLLER_H
