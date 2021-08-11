/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalmultirequest.h"
#include "captiveportalrequest.h"

#include "captiveportal.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "timersingleshot.h"
#include "networkmanager.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalMultiRequest");
}

CaptivePortalMultiRequest::CaptivePortalMultiRequest(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(CaptivePortalMultiRequest);
}

CaptivePortalMultiRequest::~CaptivePortalMultiRequest() {
  MVPN_COUNT_DTOR(CaptivePortalMultiRequest);
}

void CaptivePortalMultiRequest::run() {
  m_completed = false;
  // If we can't confirm in 30s that we are not behind
  // a captive-portal, handle this like no portal exists
  TimerSingleShot::create(this, 30 * 1000, [this]() {
    logger.error() << "CaptivePortal max timeout reached, exiting detection";
    onResult(NoPortal);
  });
  createRequest();
}

void CaptivePortalMultiRequest::createRequest() {
  NetworkManager::instance()->clearCache();
  CaptivePortalRequest* request = new CaptivePortalRequest(this);
  connect(request, &CaptivePortalRequest::completed,
          [this](CaptivePortalResult detected) {
            logger.debug() << "Captive portal detection:" << detected;
            onResult(detected);
          });

  request->run();
}

void CaptivePortalMultiRequest::onResult(CaptivePortalResult portalDetected) {
  if (m_completed) {
    return;
  }
  if (portalDetected == CaptivePortalResult::Failure) {
    logger.warning() << "Captive portal detect failed, retry!";
    TimerSingleShot::create(this, 500, [this]() { createRequest(); });
    return;
  }
  m_completed = true;
  deleteLater();
  emit completed(portalDetected);
}
