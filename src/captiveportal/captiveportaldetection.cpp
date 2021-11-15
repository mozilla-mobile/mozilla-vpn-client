/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "captiveportal.h"
#include "captiveportalmonitor.h"
#include "captiveportalnotifier.h"
#include "captiveportalmultirequest.h"
#include "constants.h"
#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalDetection");
}

CaptivePortalDetection::CaptivePortalDetection() {
  MVPN_COUNT_CTOR(CaptivePortalDetection);
}

CaptivePortalDetection::~CaptivePortalDetection() {
  MVPN_COUNT_DTOR(CaptivePortalDetection);
}

void CaptivePortalDetection::initialize() {
  m_active = SettingsHolder::instance()->captivePortalAlert();
}

void CaptivePortalDetection::stateChanged() {
  logger.debug() << "Controller/Stability state changed";

  if (!m_active) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);
  Controller::State state = vpn->controller()->state();

  if (state == Controller::StateOff) {
    // We're not connected yet - start a captivePortal Monitor
    logger.info()
        << "Not connected, starting background captive-portal Monitor";
    captivePortalBackgroundMonitor()->start();
    return;
  }
  logger.info() << "connecting, stopping background captive-portal Monitor";
  captivePortalBackgroundMonitor()->stop();

  if ((state != Controller::StateOn ||
       vpn->connectionHealth()->stability() == ConnectionHealth::Stable) &&
      state != Controller::StateConfirming) {
    logger.warning() << "No captive portal detection required";
    // Since we now reached a stable state, on the next time we have an
    // instablity check for portal again.
    m_shouldRun = true;
    return;
  }
  if (!m_shouldRun) {
    logger.debug() << "Captive Portal detection was already done for this "
                      "instability, skipping.";
    return;
  }

  detectCaptivePortal();
}

void CaptivePortalDetection::detectCaptivePortal() {
  logger.debug() << "Start the captive portal detection";

  // Quick return in case this method is called by the inspector even when the
  // feature is disabled.
  if (!m_active) {
    return;
  }
  captivePortalBackgroundMonitor()->maybeCheck();

  // The monitor must be off when detecting the captive portal.
  captivePortalMonitor()->stop();

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  // This method is called by the inspector too. Let's check the status of the
  // VPN.
  if (vpn->controller()->state() != Controller::StateOn &&
      vpn->controller()->state() != Controller::StateConfirming) {
    logger.warning() << "The VPN is not online. Ignore request.";
    return;
  }

  logger.debug() << "Captive portal detection started";

  CaptivePortalMultiRequest* request = new CaptivePortalMultiRequest(this);
  connect(request, &CaptivePortalMultiRequest::completed,
          [this](CaptivePortalResult detected) {
            logger.debug() << "Captive portal detection:" << detected;
            emit detectionCompleted(detected);
          });

  request->run();
}

void CaptivePortalDetection::settingsChanged() {
  logger.debug() << "Settings has changed";
  m_active = SettingsHolder::instance()->captivePortalAlert();

  if (!m_active) {
    captivePortalMonitor()->stop();
    captivePortalBackgroundMonitor()->stop();
  }
}

void CaptivePortalDetection::detectionCompleted(CaptivePortalResult detected) {
  logger.debug() << "Detection completed:" << detected;

  m_shouldRun = false;
  switch (detected) {
    case CaptivePortalResult::NoPortal:
    case CaptivePortalResult::Failure:
      return;
    case CaptivePortalResult::PortalDetected:
      captivePortalDetected();
      return;
  }
}

void CaptivePortalDetection::captivePortalDetected() {
  logger.debug() << "Captive portal detected!";

  // Quick return in case this method is called by the inspector even when the
  // feature is disabled.
  if (!m_active) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  if (vpn->controller()->state() == Controller::StateOn ||
      vpn->controller()->state() == Controller::StateConfirming) {
    captivePortalNotifier()->notifyCaptivePortalBlock();
  }
}

void CaptivePortalDetection::captivePortalGone() {
  logger.debug() << "Portal gone";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == MozillaVPN::StateMain &&
      vpn->controller()->state() == Controller::StateOff) {
    captivePortalNotifier()->notifyCaptivePortalUnblock();
    captivePortalMonitor()->stop();
  }
}

void CaptivePortalDetection::deactivationRequired() {
  logger.debug() << "The user wants to deactivate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->controller()->state() != Controller::StateOff) {
    vpn->deactivate();
    captivePortalMonitor()->start();
  }
}

void CaptivePortalDetection::activationRequired() {
  logger.debug() << "User wants to activate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == MozillaVPN::StateMain &&
      vpn->controller()->state() == Controller::StateOff) {
    MozillaVPN::instance()->activate();
  }
}

CaptivePortalMonitor* CaptivePortalDetection::captivePortalMonitor() {
  if (!m_captivePortalMonitor) {
    m_captivePortalMonitor = new CaptivePortalMonitor(this);

    connect(m_captivePortalMonitor, &CaptivePortalMonitor::online, this,
            &CaptivePortalDetection::captivePortalGone);
  }

  return m_captivePortalMonitor;
}

CaptivePortalMonitor* CaptivePortalDetection::captivePortalBackgroundMonitor() {
  if (!m_captivePortalBackgroundMonitor) {
    m_captivePortalBackgroundMonitor = new CaptivePortalMonitor(this);

    connect(m_captivePortalBackgroundMonitor, &CaptivePortalMonitor::online,
            MozillaVPN::instance()->controller(),
            &Controller::captivePortalGone);
    connect(m_captivePortalBackgroundMonitor, &CaptivePortalMonitor::offline,
            MozillaVPN::instance()->controller(),
            &Controller::captivePortalPresent);
  }

  return m_captivePortalBackgroundMonitor;
}

CaptivePortalNotifier* CaptivePortalDetection::captivePortalNotifier() {
  if (!m_captivePortalNotifier) {
    m_captivePortalNotifier = new CaptivePortalNotifier(this);

    connect(m_captivePortalNotifier, &CaptivePortalNotifier::activationRequired,
            this, &CaptivePortalDetection::activationRequired);

    connect(m_captivePortalNotifier,
            &CaptivePortalNotifier::deactivationRequired, this,
            &CaptivePortalDetection::deactivationRequired);
  }

  return m_captivePortalNotifier;
}
