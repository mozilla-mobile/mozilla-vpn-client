/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/dummy/dummyiaphandler.h"
#include "leakdetector.h"
#include "logger.h"

#include <QCoreApplication>

namespace {
Logger logger(LOG_IAP, "DummyIAPHandler");
DummyIAPHandler* s_instance = nullptr;
}  // namespace

DummyIAPHandler::DummyIAPHandler(QObject* parent) : IAPHandler(parent) {
  MVPN_COUNT_CTOR(DummyIAPHandler);
  Q_ASSERT(!s_instance);
  s_instance = this;
}

DummyIAPHandler::~DummyIAPHandler() {
  MVPN_COUNT_DTOR(DummyIAPHandler);
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
DummyIAPHandler* DummyIAPHandler::createInstance() {
  Q_ASSERT(!s_instance);
  new DummyIAPHandler(qApp);
  Q_ASSERT(s_instance);
  return instance();
}

// static
DummyIAPHandler* DummyIAPHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

void DummyIAPHandler::nativeRegisterProducts() {}

void DummyIAPHandler::nativeStartSubscription(Product* product) {}
