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

CaptivePortalNotifier::CaptivePortalNotifier(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(CaptivePortalNotifier);

  connect(SystemTrayHandler::instance(),
          &SystemTrayHandler::notificationClicked, this,
          &CaptivePortalNotifier::notificationClicked);
}

CaptivePortalNotifier::~CaptivePortalNotifier() {
  MVPN_COUNT_DTOR(CaptivePortalNotifier);
}

void CaptivePortalNotifier::notifyCaptivePortalBlock() {
  logger.log() << "Captive portal block notify";
  SystemTrayHandler::instance()->captivePortalBlockNotificationRequired();
}

void CaptivePortalNotifier::notifyCaptivePortalUnblock() {
  logger.log() << "Captive portal unblock notify";
  SystemTrayHandler::instance()->captivePortalUnblockNotificationRequired();
}

void CaptivePortalNotifier::notificationClicked(
    SystemTrayHandler::Message message) {
  logger.log() << "Notification clicked";

  switch (message) {
    case SystemTrayHandler::CaptivePortalBlock:
      emit deactivationRequired();
      break;

    case SystemTrayHandler::CaptivePortalUnblock:
      emit activationRequired();
      break;

    default:
      logger.warning() << "Ignore message";
      break;
  }
}
