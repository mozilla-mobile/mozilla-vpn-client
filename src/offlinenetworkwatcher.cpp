/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "offlinenetworkwatcher.h"

#include "leakdetector.h"

OfflineNetworkWatcher::OfflineNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MZ_COUNT_CTOR(OfflineNetworkWatcher);
}

OfflineNetworkWatcher::~OfflineNetworkWatcher() {
  MZ_COUNT_DTOR(OfflineNetworkWatcher);
}

void OfflineNetworkWatcher::initialize() {}
