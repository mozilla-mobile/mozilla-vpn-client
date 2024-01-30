/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNSERVERCOUNTRYMODEL_H
#define VPNSERVERCOUNTRYMODEL_H

#include <QQmlEngine>

#include "models/servercountrymodel.h"
#include "mozillavpn.h"

struct VPNServerCountryModel {
  Q_GADGET
  QML_FOREIGN(ServerCountryModel)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ServerCountryModel* create(QQmlEngine* qmlEngine,
                                    QJSEngine* jsEngine) {
    return MozillaVPN::instance()->serverCountryModel();
  }
};

#endif  // VPNSERVERCOUNTRYMODEL_H
