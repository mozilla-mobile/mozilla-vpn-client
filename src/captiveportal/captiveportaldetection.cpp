/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "captiveportal.h"
#include "captiveportaldetectionimpl.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowscaptiveportaldetection.h"
#endif

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalDetection");
}

CaptivePortalDetection::CaptivePortalDetection() {
  MVPN_COUNT_CTOR(CaptivePortalDetection);

  connect(&m_captivePortalNotifier,
          &CaptivePortalNotifier::notificationCaptivePortalBlockCompleted, this,
          &CaptivePortalDetection::notificationCaptivePortalBlockCompleted);

  connect(&m_captivePortalNotifier,
          &CaptivePortalNotifier::notificationCaptivePortalUnblockCompleted, this,
          &CaptivePortalDetection::notificationCaptivePortalUnblockCompleted);

  connect(&m_captivePortalMonitor, &CaptivePortalMonitor::online, this,
          &CaptivePortalDetection::captivePortalGone);
}

CaptivePortalDetection::~CaptivePortalDetection() {
  MVPN_COUNT_DTOR(CaptivePortalDetection);
}

void CaptivePortalDetection::initialize() {
  m_active = SettingsHolder::instance()->captivePortalAlert();
}

void CaptivePortalDetection::stateChanged() {
  logger.log() << "Controller/Stability state changed";

  if (!m_active) {
    return;
  }

  // Something has changed. Stopping the monitoring.
  m_captivePortalMonitor.stop();

  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn ||
      MozillaVPN::instance()->connectionHealth()->stability() ==
          ConnectionHealth::Stable) {
    logger.log() << "No captive portal detection required";
    m_impl.reset();
    return;
  }

  logger.log() << "Start the captive portal detection";
#ifdef MVPN_WINDOWS
  m_impl.reset(new WindowsCaptivePortalDetection());
#else
  logger.log() << "This platform does not support captive portal detection yet";
  return;
#endif

  Q_ASSERT(m_impl);

  connect(m_impl.get(), &CaptivePortalDetectionImpl::detectionCompleted, this,
          &CaptivePortalDetection::detectionCompleted);

  m_impl->start();
}

void CaptivePortalDetection::settingsChanged() {
  logger.log() << "Settings has changed";
  m_active = SettingsHolder::instance()->captivePortalAlert();

  if (!m_active) {
    m_captivePortalMonitor.stop();
    m_impl.reset();
  }
}

void CaptivePortalDetection::detectionCompleted(bool detected) {
  logger.log() << "Detection completed:" << detected;

  m_impl.reset();

  if (!detected) {
    return;
  }

  if (MozillaVPN::instance()->controller()->state() == Controller::StateOn) {
    m_captivePortalNotifier.notifyCaptivePortalBlock();
  }
}

void CaptivePortalDetection::captivePortalGone() {
  logger.log() << "Portal gone";

  if (MozillaVPN::instance()->controller()->state() == Controller::StateOff) {
    m_captivePortalNotifier.notifyCaptivePortalUnblock();
  }
}

void CaptivePortalDetection::notificationCaptivePortalBlockCompleted(
    bool disconnectionRequested) {
  logger.log() << "User informed. The disconnection request status:"
               << disconnectionRequested;

  if (!disconnectionRequested) {
    return;
  }

  MozillaVPN::instance()->deactivate();

  m_captivePortalMonitor.start();
}

void CaptivePortalDetection::notificationCaptivePortalUnblockCompleted(
    bool connectionRequested) {
  logger.log() << "User informed. The connection request status:"
               << connectionRequested;

  if (!connectionRequested) {
    return;
  }

  MozillaVPN::instance()->activate();
}