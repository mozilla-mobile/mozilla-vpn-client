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

void PurchaseWebHandler::startSubscription(const QString& productIdentifier) {
  ProductsHandler* productsHandler = ProductsHandler::instance();
  Q_ASSERT(productsHandler->hasProductsRegistered());

  ProductsHandler::Product* product =
      productsHandler->findProduct(productIdentifier);
  Q_ASSERT(product);

  if (m_subscriptionState != eInactive) {
    logger.warning() << "We're already subscribing.";
    return;
  }
  m_subscriptionState = eActive;
  logger.debug() << "Starting the subscription";
}

void PurchaseWebHandler::startRestoreSubscription() {
  logger.error() << "Restore not implemented!";
  emit subscriptionFailed();
}

void PurchaseWebHandler::nativeRegisterProducts() {
  logger.error()
      << "nativeRegisterProducts should not be called for PurchaseWebHandler";
  Q_ASSERT(false);
}
