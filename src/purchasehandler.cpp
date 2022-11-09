/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "purchasehandler.h"
#include "constants.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QScopeGuard>

#ifdef MVPN_IOS
#  include "platforms/ios/iosiaphandler.h"
#elif MVPN_ANDROID
#  include "platforms/android/androidiaphandler.h"
#elif MVPN_WASM
#  include "platforms/wasm/wasmiaphandler.h"
#else
#  include "platforms/dummy/dummyiaphandler.h"
#endif

namespace {
Logger logger(LOG_IAP, "PurchaseHandler");
PurchaseHandler* s_instance = nullptr;
}  // namespace

// static
PurchaseHandler* PurchaseHandler::createInstance() {
  Q_ASSERT(!s_instance);
#ifdef MVPN_IOS
  new IOSPurchaseHandler(qApp);
#elif MVPN_ANDROID
  new AndroidPurchaseHandler(qApp);
#elif MVPN_WASM
  new WasmPurchaseHandler(qApp);
#else
  new DummyPurchaseHandler(qApp);
#endif
  Q_ASSERT(s_instance);
  return instance();
}

// static
PurchaseHandler* PurchaseHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

PurchaseHandler::PurchaseHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(PurchaseHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

PurchaseHandler::~PurchaseHandler() {
  MVPN_COUNT_DTOR(PurchaseHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void PurchaseHandler::startSubscription(const QString& productIdentifier) {
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

void PurchaseHandler::startRestoreSubscription() {
  logger.debug() << "Starting the restore of the subscription";

#ifdef MVPN_IOS
  nativeRestoreSubscription();
#else
  logger.error() << "Restore not implemented!";
  emit subscriptionFailed();
#endif
}

void PurchaseHandler::stopSubscription() {
  logger.debug() << "Stop subscription";
  m_subscriptionState = eInactive;
}

void PurchaseHandler::subscribe(const QString& productIdentifier) {
  logger.debug() << "Subscription required";
  m_currentSKU = productIdentifier;
  emit subscriptionStarted(productIdentifier);
}

void PurchaseHandler::restore() {
  logger.debug() << "Restore purchase";
  emit restoreSubscriptionStarted();
}
