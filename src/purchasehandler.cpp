/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "purchasehandler.h"

#include <QCoreApplication>

#include "logging/logger.h"
#include "utilities/leakdetector.h"

#ifdef MZ_IOS
#  include "platforms/ios/iosiaphandler.h"
#elif MZ_ANDROID
#  include "platforms/android/androidiaphandler.h"
#elif MZ_WASM
#  include "platforms/wasm/wasmiaphandler.h"
#else
#  include "purchasewebhandler.h"
#endif

namespace {
Logger logger("PurchaseHandler");
PurchaseHandler* s_instance = nullptr;
}  // namespace

// static
PurchaseHandler* PurchaseHandler::createInstance() {
  Q_ASSERT(!s_instance);
#ifdef MZ_IOS
  new IOSIAPHandler(qApp);
#elif MZ_ANDROID
  new AndroidIAPHandler(qApp);
#elif MZ_WASM
  new WasmIAPHandler(qApp);
#else
  new PurchaseWebHandler(qApp);
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
  MZ_COUNT_CTOR(PurchaseHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

PurchaseHandler::~PurchaseHandler() {
  MZ_COUNT_DTOR(PurchaseHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void PurchaseHandler::subscribe(const QString& productIdentifier) {
  logger.debug() << "Subscription required" << productIdentifier;
  m_currentSKU = productIdentifier;
  emit subscriptionStarted(productIdentifier);
}

void PurchaseHandler::stopSubscription() {
  logger.debug() << "Stop subscription";
  m_subscriptionState = eInactive;
}

void PurchaseHandler::restore() {
  logger.debug() << "Restore purchase";
  emit restoreSubscriptionStarted();
}
