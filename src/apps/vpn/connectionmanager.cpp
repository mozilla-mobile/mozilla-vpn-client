/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionmanager.h"

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"

namespace {
Logger logger("Connection Manager");
}

ConnectionManager::ConnectionManager() {
  MZ_COUNT_CTOR(ConnectionManager);
  
}

ConnectionManager::~ConnectionManager() {
  MZ_COUNT_DTOR(ConnectionManager);
}

void ConnectionManager::initialize() {
  logger.debug() << "Initializing the connection manager";
}
