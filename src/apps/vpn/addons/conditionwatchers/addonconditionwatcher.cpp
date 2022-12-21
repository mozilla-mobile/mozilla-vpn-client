/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcher.h"

#include "leakdetector.h"

AddonConditionWatcher::AddonConditionWatcher(QObject* parent)
    : QObject(parent) {
  MZ_COUNT_CTOR(AddonConditionWatcher);
}

AddonConditionWatcher::~AddonConditionWatcher() {
  MZ_COUNT_DTOR(AddonConditionWatcher);
}
