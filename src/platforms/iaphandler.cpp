
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/iaphandler.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_IAP, "IAPHandler");
}  // anonymous namespace

IAPHandler::IAPHandler(QObject* parent)
    : QAbstractListModel(parent) {
  MVPN_COUNT_CTOR(IAPHandler);
}

IAPHandler::~IAPHandler() {
  MVPN_COUNT_DTOR(IAPHandler);
}