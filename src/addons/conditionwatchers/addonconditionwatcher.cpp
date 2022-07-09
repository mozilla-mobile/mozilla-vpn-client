/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcher.h"
#include "leakdetector.h"

AddonConditionWatcher::AddonConditionWatcher(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(AddonConditionWatcher);
}

AddonConditionWatcher::~AddonConditionWatcher() {
  MVPN_COUNT_DTOR(AddonConditionWatcher);
}
