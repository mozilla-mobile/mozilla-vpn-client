/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNSUBSCRIPTIONDATA_H
#define VPNSUBSCRIPTIONDATA_H

#include <QQmlEngine>

#include "models/subscriptiondata.h"
#include "mozillavpn.h"

struct VPNSubscriptionData {
  Q_GADGET
  QML_FOREIGN(SubscriptionData)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static SubscriptionData* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->subscriptionData();
  }
};

#endif  // VPNSUBSCRIPTIONDATA_H
