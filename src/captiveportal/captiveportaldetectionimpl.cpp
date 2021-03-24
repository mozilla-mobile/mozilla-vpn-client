/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetectionimpl.h"
#include "captiveportalmultirequest.h"
#include "captiveportalresult.h"

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
  logger.log() << "Captive portal detection started";

  CaptivePortalMultiRequest* request = new CaptivePortalMultiRequest(this);
  connect(request, &CaptivePortalMultiRequest::completed,
          [this](CaptivePortalResult detected) {
            logger.log() << "Captive portal detection:" << detected;
            emit detectionCompleted(detected);
          });

  request->run();
}
