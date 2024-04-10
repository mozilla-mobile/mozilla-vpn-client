/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmnetworkwatcher.h"

#include "logger.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("WasmNetworkWatcher");
}

WasmNetworkWatcher::WasmNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MZ_COUNT_CTOR(WasmNetworkWatcher);
}

WasmNetworkWatcher::~WasmNetworkWatcher() { MZ_COUNT_DTOR(WasmNetworkWatcher); }

void WasmNetworkWatcher::initialize() { logger.debug() << "initialize"; }

void WasmNetworkWatcher::start() {
  logger.debug() << "actived";
  emit unsecuredNetwork("WifiName", "NetworkID");
}
