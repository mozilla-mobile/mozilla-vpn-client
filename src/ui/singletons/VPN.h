/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPN_H
#define VPN_H

#include <QQmlEngine>

#include "mozillavpn.h"

struct VPN {
  Q_GADGET
  QML_FOREIGN(MozillaVPN)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static MozillaVPN* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance();
  }
};

#endif  // VPN_H
