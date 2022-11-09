/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "purchasewebhandler.h"
#include "leakdetector.h"
#include "logger.h"

#include <QCoreApplication>

namespace {
Logger logger(LOG_IAP, "PurchaseWebHandler");
}  // namespace

PurchaseWebHandler::PurchaseWebHandler(QObject* parent)
    : PurchaseHandler(parent) {
  MVPN_COUNT_CTOR(PurchaseWebHandler);
}

PurchaseWebHandler::~PurchaseWebHandler() {
  MVPN_COUNT_DTOR(PurchaseWebHandler);
}

void PurchaseWebHandler::nativeStartSubscription(
    ProductsHandler::Product* product) {
  Q_UNUSED(product)
}
void PurchaseWebHandler::nativeRestoreSubscription() {}
