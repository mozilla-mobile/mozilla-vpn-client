/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNPURCHASE_H
#define VPNPURCHASE_H

#include <QQmlEngine>

#include "mozillavpn.h"
#include "purchasehandler.h"

struct VPNPurchase {
  Q_GADGET
  QML_FOREIGN(PurchaseHandler)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static PurchaseHandler* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return PurchaseHandler::instance();
  }
};

#endif  // VPNPURCHASE_H
