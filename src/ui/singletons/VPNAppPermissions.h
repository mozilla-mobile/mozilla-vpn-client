/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNAPPPERMISSIONS_H
#define VPNAPPPERMISSIONS_H

#include <QQmlEngine>

#include "apppermission.h"

struct VPNAppPermissions {
  Q_GADGET
  QML_FOREIGN(AppPermission)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static AppPermission* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return AppPermission::instance();
  }
};

#endif  // VPNAPPPERMISSIONS_H
