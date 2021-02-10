/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalnotifier.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "systemtrayhandler.h"

namespace {
Logger logger(LOG_NETWORKING, "CaptivePortalNotifier");
}

CaptivePortalNotifier::CaptivePortalNotifier() {
  MVPN_COUNT_CTOR(CaptivePortalNotifier);

  m_notifyTimer.setSingleShot(true);
  connect(&m_notifyTimer, &QTimer::timeout, this,
          &CaptivePortalNotifier::notifyTimerExpired);

  connect(SystemTrayHandler::instance(), &SystemTrayHandler::messageClicked,
          this, &CaptivePortalNotifier::messageClicked);
}

CaptivePortalNotifier::~CaptivePortalNotifier() {
  MVPN_COUNT_DTOR(CaptivePortalNotifier);
}

void CaptivePortalNotifier::notify() {
  logger.log() << "Captive portal notify";

  m_notifyTimer.start(Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  SystemTrayHandler::instance()->captivePortalNotificationRequired();
}

void CaptivePortalNotifier::messageClicked() {
  logger.log() << "Message clicked";

  if (!m_notifyTimer.isActive()) {
    logger.log() << "The message is not for us. Let's ignore it.";
    return;
  }

  m_notifyTimer.stop();
  emit notificationCompleted(true);
}

void CaptivePortalNotifier::notifyTimerExpired() {
  logger.log() << "Notify timer expired";
  emit notificationCompleted(false);
}
