/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNPRODUCTS_H
#define VPNPRODUCTS_H

#include <QQmlEngine>

#include "productshandler.h"

struct VPNProducts {
  Q_GADGET
  QML_FOREIGN(ProductsHandler)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ProductsHandler* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return ProductsHandler::instance();
  }
};

#endif  // VPNPRODUCTS_H
