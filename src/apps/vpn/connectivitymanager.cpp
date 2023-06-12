/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectivitymanager.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("Connectivity Manager");
}

ConnectivityManager::ConnectivityManager() {
  MZ_COUNT_CTOR(ConnectivityManager);

}

ConnectivityManager::~ConnectivityManager() {
  MZ_COUNT_DTOR(ConnectivityManager);
}

void ConnectivityManager::initialize() {
  logger.debug() << "Initializing the connectivity manager";
}


