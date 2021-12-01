/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskinitializeadjust.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "adjust/adjusthandler.h"

namespace {
Logger logger(LOG_MAIN, "TaskInitializeAdjust");
}

TaskInitializeAdjust::TaskInitializeAdjust() : Task("TaskInitializeAdjust") {
  MVPN_COUNT_CTOR(TaskInitializeAdjust);
}

TaskInitializeAdjust::~TaskInitializeAdjust() {
  MVPN_COUNT_DTOR(TaskInitializeAdjust);
}

void TaskInitializeAdjust::run() { AdjustHandler::initialize(); }
