/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalnotifier.h"

#include "context/qmlengineholder.h"
#include "logging/logger.h"
#include "notificationhandler.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("CaptivePortalNotifier");
}

CaptivePortalNotifier::CaptivePortalNotifier(QObject* parent)
    : QObject(parent) {
  MZ_COUNT_CTOR(CaptivePortalNotifier);

  connect(NotificationHandler::instance(),
          &NotificationHandler::notificationClicked, this,
          &CaptivePortalNotifier::notificationClicked);
}

CaptivePortalNotifier::~CaptivePortalNotifier() {
  MZ_COUNT_DTOR(CaptivePortalNotifier);
}

void CaptivePortalNotifier::notifyCaptivePortalBlock() {
  logger.debug() << "Captive portal block notify";
  NotificationHandler::instance()->captivePortalBlockNotificationRequired();
}

void CaptivePortalNotifier::notifyCaptivePortalUnblock() {
  logger.debug() << "Captive portal unblock notify";
  NotificationHandler::instance()->captivePortalUnblockNotificationRequired();
}

void CaptivePortalNotifier::notificationClicked(
    NotificationHandler::Message message) {
  logger.debug() << "Notification clicked";

  if (message == NotificationHandler::CaptivePortalUnblock) {
    emit activationRequired();
  }
  QmlEngineHolder::instance()->showWindow();
}
