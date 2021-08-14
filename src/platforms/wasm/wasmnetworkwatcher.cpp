/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_NETWORKING, "WasmNetworkWatcher");
}

WasmNetworkWatcher::WasmNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(WasmNetworkWatcher);
}

WasmNetworkWatcher::~WasmNetworkWatcher() {
  MVPN_COUNT_DTOR(WasmNetworkWatcher);
}

void WasmNetworkWatcher::initialize() { logger.debug() << "initialize"; }

void WasmNetworkWatcher::start() {
  logger.debug() << "actived";
  emit unsecuredNetwork("WifiName", "NetworkID");
}
