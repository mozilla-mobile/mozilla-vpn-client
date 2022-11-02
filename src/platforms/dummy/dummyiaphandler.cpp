/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/dummy/dummyiaphandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#include <QCoreApplication>
#include <QTimer>

namespace {
Logger logger(LOG_IAP, "DummyIAPHandler");
}  // namespace

DummyIAPHandler::DummyIAPHandler(QObject* parent) : IAPHandler(parent) {
  MVPN_COUNT_CTOR(DummyIAPHandler);
}

DummyIAPHandler::~DummyIAPHandler() { MVPN_COUNT_DTOR(DummyIAPHandler); }

void DummyIAPHandler::nativeRegisterProducts() {
  QTimer::singleShot(200, this,
                     [this]() { emit productsRegistrationCompleted(); });
}

void DummyIAPHandler::nativeStartSubscription(Product* product) {
  Q_ASSERT(product->m_name == "web");
  MozillaVPN::instance()->logout();
  MozillaVPN::instance()->authenticateWithType(
      MozillaVPN::AuthenticationType::AuthenticationInBrowser);
}

void DummyIAPHandler::nativeRestoreSubscription() {}
