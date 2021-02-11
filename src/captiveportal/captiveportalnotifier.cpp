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

void CaptivePortalNotifier::notifyCaptivePortalBlock() {
  logger.log() << "Captive portal block notify";

  m_type = Block;
  m_notifyTimer.start(Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  SystemTrayHandler::instance()->captivePortalBlockNotificationRequired();
}

void CaptivePortalNotifier::notifyCaptivePortalUnblock() {
  logger.log() << "Captive portal unblock notify";

  m_type = Unblock;
  m_notifyTimer.start(Constants::CAPTIVE_PORTAL_ALERT_MSEC);
  SystemTrayHandler::instance()->captivePortalUnblockNotificationRequired();
}

void CaptivePortalNotifier::messageClicked() {
  logger.log() << "Message clicked";

  if (!m_notifyTimer.isActive()) {
    logger.log() << "The message is not for us. Let's ignore it.";
    return;
  }

  m_notifyTimer.stop();
  emitSignal(true /* user accepted */);
}

void CaptivePortalNotifier::emitSignal(bool userAccepted) {
  switch (m_type) {
    case Block:
      m_type = Unset;
      emit notificationCaptivePortalBlockCompleted(userAccepted);
      break;

    case Unblock:
      m_type = Unset;
      emit notificationCaptivePortalUnblockCompleted(userAccepted);
      break;

    default:
      break;
  }
}

void CaptivePortalNotifier::notifyTimerExpired() {
  logger.log() << "Notify timer expired";
  emitSignal(false /* user accepted */);
}
