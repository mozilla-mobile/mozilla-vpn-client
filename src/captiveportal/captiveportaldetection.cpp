/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportaldetection.h"

#include "captiveportal.h"
#include "captiveportaldetectionimpl.h"
#include "captiveportalnotifier.h"
#include "connectionhealth.h"
#include "controller.h"
#include "logging/logger.h"
#include "mozillavpn.h"
#include "navigator/navigator.h"
#include "networkwatcher.h"
#include "settings/settingsholder.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("CaptivePortalDetection");
}

CaptivePortalDetection::CaptivePortalDetection() {
  MZ_COUNT_CTOR(CaptivePortalDetection);
}

CaptivePortalDetection::~CaptivePortalDetection() {
  MZ_COUNT_DTOR(CaptivePortalDetection);
}

void CaptivePortalDetection::initialize() {
  m_active = SettingsHolder::instance()->captivePortalAlert();
  const auto networkWatcher = MozillaVPN::instance()->networkWatcher();
  connect(networkWatcher, &NetworkWatcher::networkChange, this,
          &CaptivePortalDetection::networkChanged);

  connect(MozillaVPN::instance()->controller(), &Controller::stateChanged, this,
          &CaptivePortalDetection::stateChanged);

  connect(MozillaVPN::instance()->connectionHealth(),
          &ConnectionHealth::stabilityChanged, this,
          &CaptivePortalDetection::stateChanged);

  connect(SettingsHolder::instance(),
          &SettingsHolder::captivePortalAlertChanged, this,
          &CaptivePortalDetection::settingsChanged);
}

void CaptivePortalDetection::networkChanged() {
  MozillaVPN* vpn = MozillaVPN::instance();

  Controller::State state = vpn->controller()->state();
  if (state != Controller::StateOn && state != Controller::StateConnecting &&
      state != Controller::StateCheckSubscription &&
      state != Controller::StateConfirming) {
    // Network Changed but we're not connected, no need to test for captive
    // portal
    return;
  }
  logger.debug() << "Current Network Changed, checking for Portal";
  detectCaptivePortal();
}

void CaptivePortalDetection::stateChanged() {
  logger.debug() << "Controller/Stability state changed";

  if (!m_active) {
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  Controller::State state = vpn->controller()->state();

  if (state == Controller::StateOff) {
    // We're not connected yet - do not check for captive-portal
    logger.info() << "Not connected, do not check for captive-portal";
    return;
  }

  if ((state != Controller::StateOn ||
       vpn->connectionHealth()->stability() == ConnectionHealth::Stable) &&
      state != Controller::StateConnecting &&
      state != Controller::StateCheckSubscription &&
      state != Controller::StateConfirming) {
    logger.warning() << "No captive portal detection required";
    m_impl.reset();
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

  MozillaVPN* vpn = MozillaVPN::instance();

  // This method is called by the inspector too. Let's check the status of the
  // VPN.
  Controller::State state = vpn->controller()->state();
  if (state != Controller::StateOn && state != Controller::StateConnecting &&
      state != Controller::StateConfirming) {
    logger.warning() << "The VPN is not online. Ignore request.";
    return;
  }

  logger.debug() << "Captive portal detection started";

#if defined(MZ_LINUX) || defined(MZ_MACOS) || defined(MZ_WINDOWS)
  m_impl.reset(new CaptivePortalDetectionImpl());
#else
  logger.warning()
      << "This platform does not support captive portal detection yet";
  return;
#endif

  Q_ASSERT(m_impl);

  connect(m_impl.get(), &CaptivePortalDetectionImpl::detectionCompleted, this,
          &CaptivePortalDetection::detectionCompleted);

  m_impl->start();
}

void CaptivePortalDetection::settingsChanged() {
  logger.debug() << "Settings has changed";
  m_active = SettingsHolder::instance()->captivePortalAlert();

  if (!m_active) {
    m_impl.reset();
  }
}

void CaptivePortalDetection::detectionCompleted(
    CaptivePortalRequest::CaptivePortalResult detected) {
  logger.debug() << "Detection completed:" << detected;
  m_impl.reset();
  m_shouldRun = false;
  switch (detected) {
    case CaptivePortalRequest::CaptivePortalResult::NoPortal:
    case CaptivePortalRequest::CaptivePortalResult::Failure:
      return;
    case CaptivePortalRequest::CaptivePortalResult::PortalDetected:
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
  emit captivePortalPresent();
  Navigator::instance()->requestScreen(MozillaVPN::ScreenCaptivePortal);

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->controller()->state() == Controller::StateOn) {
    captivePortalNotifier()->notifyCaptivePortalBlock();
  }
}

void CaptivePortalDetection::captivePortalGone() {
  logger.debug() << "Portal gone";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == App::StateMain &&
      vpn->controller()->state() == Controller::StateOff) {
    captivePortalNotifier()->notifyCaptivePortalUnblock();
  }
}

void CaptivePortalDetection::deactivationRequired() {
  logger.debug() << "The user wants to deactivate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();

  if (vpn->controller()->state() != Controller::StateOff) {
    vpn->deactivate();
  }
}

void CaptivePortalDetection::activationRequired() {
  logger.debug() << "User wants to activate the vpn";

  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn->state() == App::StateMain &&
      vpn->controller()->state() == Controller::StateOff) {
    vpn->controller()->captivePortalGone();
    vpn->activate();
  }
}

CaptivePortalNotifier* CaptivePortalDetection::captivePortalNotifier() {
  if (!m_captivePortalNotifier) {
    m_captivePortalNotifier = new CaptivePortalNotifier(this);

    connect(m_captivePortalNotifier, &CaptivePortalNotifier::activationRequired,
            this, &CaptivePortalDetection::activationRequired);
  }

  return m_captivePortalNotifier;
}
