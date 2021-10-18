/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/android/androidnotificationhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "androidcontroller.h"

namespace {
Logger logger(LOG_ANDROID, "AndroidNotificationHandler");
}

AndroidNotificationHandler::AndroidNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MVPN_COUNT_CTOR(AndroidNotificationHandler);
}
AndroidNotificationHandler::~AndroidNotificationHandler() {
  MVPN_COUNT_DTOR(AndroidNotificationHandler);
}

void AndroidNotificationHandler::notify(NotificationHandler::Message type,
                                        const QString& title,
                                        const QString& message, int timerMsec) {
  Q_UNUSED(type);
  logger.debug() << "Send notification - " << message;
  AndroidController::instance()->setNotificationText(title, message,
                                                     timerMsec / 1000);
}
