/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"
#include "captiveportal.h"
#include "captiveportaldetectionimpl.h"
#include "captiveportalmonitor.h"
#include "captiveportalnotifier.h"
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

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  if ((vpn->controller()->state() != Controller::StateOn ||
       vpn->connectionHealth()->stability() == ConnectionHealth::Stable) &&
      vpn->controller()->state() != Controller::StateConfirming) {
    logger.warning() << "No captive portal detection required";
    m_impl.reset();
    // Since we now reached a stable state, on the next time we have an
    // instablity check for portal again.
    m_shouldRun = true;
    return;
  }
  if (!m_shouldRun) {
    logger.log() << "Captive Portal detection was already done for this "
                    "instability, skipping.";
    return;
  }

  detectCaptivePortal();
}

void CaptivePortalDetection::detectCaptivePortal() {
  logger.log() << "Start the captive portal detection";

  // Quick return in case this method is called by the inspector even when the
  // feature is disabled.
  if (!m_active) {
    return;
  }

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

#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS)
  m_impl.reset(new CaptivePortalDetectionImpl());
#else
  logger.warning() << "This platform does not support captive portal detection yet";
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
    captivePortalMonitor()->stop();
    m_impl.reset();
  }
}

void CaptivePortalDetection::detectionCompleted(CaptivePortalResult detected) {
  logger.log() << "Detection completed:" << detected;

  m_impl.reset();
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
  logger.log() << "Captive portal detected!";

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
  logger.log() << "Portal gone";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == MozillaVPN::StateMain &&
      vpn->controller()->state() == Controller::StateOff) {
    captivePortalNotifier()->notifyCaptivePortalUnblock();
    captivePortalMonitor()->stop();
  }
}

void CaptivePortalDetection::deactivationRequired() {
  logger.log() << "The user wants to deactivate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->controller()->state() != Controller::StateOff) {
    vpn->deactivate();
    captivePortalMonitor()->start();
  }
}

void CaptivePortalDetection::activationRequired() {
  logger.log() << "User wants to activate the vpn";

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
