/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "purchaseiaphandler.h"

#include "feature.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("PurchaseIAPHandler");
PurchaseIAPHandler* s_instance = nullptr;
}  // namespace

// static
PurchaseIAPHandler* PurchaseIAPHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

PurchaseIAPHandler::PurchaseIAPHandler(QObject* parent)
    : PurchaseHandler(parent) {
  MZ_COUNT_CTOR(PurchaseIAPHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

PurchaseIAPHandler::~PurchaseIAPHandler() {
  MZ_COUNT_DTOR(PurchaseIAPHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void PurchaseIAPHandler::startSubscription(const QString& productIdentifier) {
  ProductsHandler* productsHandler = ProductsHandler::instance();
  Q_ASSERT(productsHandler->hasProductsRegistered());

  ProductsHandler::Product* product =
      productsHandler->findProduct(productIdentifier);
  Q_ASSERT(product);

  if (m_subscriptionState != eInactive) {
    logger.warning() << "No multiple IAP!";
    return;
  }
  m_subscriptionState = eActive;
  logger.debug() << "Starting the subscription";
  nativeStartSubscription(product);
}

void PurchaseIAPHandler::startRestoreSubscription() {
  logger.debug() << "Starting the restore of the subscription";

#ifdef MZ_IOS
  nativeRestoreSubscription();
#else
  logger.error() << "Restore not implemented!";
  emit subscriptionFailed();
#endif
}
