/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "captiveportal.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalDetection");
}

CaptivePortalDetection::CaptivePortalDetection() {
  MVPN_COUNT_CTOR(CaptivePortalDetection);

  connect(&m_timer, &QTimer::timeout, this,
          &CaptivePortalDetection::detectCaptivePortal);
}

CaptivePortalDetection::~CaptivePortalDetection() {
  MVPN_COUNT_DTOR(CaptivePortalDetection);
}

void CaptivePortalDetection::initialize() {
  m_active = SettingsHolder::instance()->captivePortalAlert();
}

void CaptivePortalDetection::stateChanged() {
  logger.log() << "Controller/Stability state changed";

  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn ||
      MozillaVPN::instance()->connectionHealth()->stability() == ConnectionHealth::Stable) {
    logger.log() << "No captive portal detection required";
    m_timer.stop();
    return;
  }

  logger.log() << "Start the captive portal detection";

  m_timer.start(Constants::CAPTIVEPORTAL_REQUEST_TIMEOUT_MSEC);
  detectCaptivePortal();
}

void CaptivePortalDetection::settingsChanged() {
  logger.log() << "Settings has changed";
  m_active = SettingsHolder::instance()->captivePortalAlert();
}

void CaptivePortalDetection::detectCaptivePortal() {
  logger.log() << "Detecting captive portal - status:" << m_active;

  if (!m_active) {
    return;
  }

  // TODO
}
