/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNDEVICEMODEL_H
#define VPNDEVICEMODEL_H

#include <QQmlEngine>

#include "models/devicemodel.h"
#include "mozillavpn.h"

struct VPNDeviceModel {
  Q_GADGET
  QML_FOREIGN(DeviceModel)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static DeviceModel* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->deviceModel();
  }
};

#endif  // VPNDEVICEMODEL_H
