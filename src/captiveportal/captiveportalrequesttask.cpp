/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalrequesttask.h"

#include <QTimer>

#include "captiveportal.h"
#include "captiveportalrequest.h"
#include "logger.h"
#include "networkmanager.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("CaptivePortalRequestTask");
}

CaptivePortalRequestTask::CaptivePortalRequestTask(bool retryOnFailure)
    : Task("CaptivePortalRequestTask"), m_retryOnFailure(retryOnFailure) {
  MZ_COUNT_CTOR(CaptivePortalRequestTask);
}

CaptivePortalRequestTask::~CaptivePortalRequestTask() {
  MZ_COUNT_DTOR(CaptivePortalRequestTask);
}

void CaptivePortalRequestTask::run() {
  // If we can't confirm in 30s that we are not behind
  // a captive-portal, handle this like no portal exists
  QTimer::singleShot(30 * 1000, this, [this]() {
    logger.error() << "CaptivePortal max timeout reached, exiting detection";
    onResult(CaptivePortalRequest::CaptivePortalResult::NoPortal);
  });
  createRequest();
}

void CaptivePortalRequestTask::createRequest() {
  NetworkManager::instance()->clearCache();
  CaptivePortalRequest* request = new CaptivePortalRequest(this);
  connect(request, &CaptivePortalRequest::completed, this,
          [this](CaptivePortalRequest::CaptivePortalResult detected) {
            logger.debug() << "Captive portal detection:" << detected;
            onResult(detected);
          });

  request->run();
}

void CaptivePortalRequestTask::onResult(
    CaptivePortalRequest::CaptivePortalResult portalDetected) {
  if (m_completed) {
    return;
  }
  if (portalDetected == CaptivePortalRequest::CaptivePortalResult::Failure &&
      m_retryOnFailure) {
    logger.warning() << "Captive portal detect failed, retry!";
    QTimer::singleShot(500, this, [this]() { createRequest(); });
    return;
  }
  m_completed = true;

  emit operationCompleted(portalDetected);
  emit completed();
}
