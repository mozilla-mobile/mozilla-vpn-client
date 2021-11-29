/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetectionimpl.h"
#include "captiveportalrequesttask.h"
#include "taskscheduler.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_NETWORKING, "CaptivePortalDetectionImpl");
}

CaptivePortalDetectionImpl::CaptivePortalDetectionImpl() {
  MVPN_COUNT_CTOR(CaptivePortalDetectionImpl);
}

CaptivePortalDetectionImpl::~CaptivePortalDetectionImpl() {
  MVPN_COUNT_DTOR(CaptivePortalDetectionImpl);
}

void CaptivePortalDetectionImpl::start() {
  logger.debug() << "Captive portal detection started";

  CaptivePortalRequestTask* task = new CaptivePortalRequestTask();
  connect(task, &CaptivePortalRequestTask::operationCompleted,
          [this](CaptivePortalRequest::CaptivePortalResult detected) {
            logger.debug() << "Captive portal detection:" << detected;
            emit detectionCompleted(detected);
          });

  TaskScheduler::scheduleTask(task);
}
