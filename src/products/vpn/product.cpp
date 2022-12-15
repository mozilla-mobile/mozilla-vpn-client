/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "product.h"

#include "leakdetector.h"
#include "moduleholder.h"
#include "modules/vpn.h"

Product::Product(QObject* parent) {
  MVPN_COUNT_CTOR(Product);

  ModuleHolder::instance()->registerModule("vpn", new ModuleVPN(parent));
}

Product::~Product() { MVPN_COUNT_DTOR(Product); }
