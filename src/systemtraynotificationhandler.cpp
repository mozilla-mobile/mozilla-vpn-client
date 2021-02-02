/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systemtraynotificationhandler.h"
#include "leakdetector.h"
#include "systemtrayhandler.h"
#include "constants.h"

SystemTrayNotificationHandler::SystemTrayNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MVPN_COUNT_CTOR(SystemTrayNotificationHandler);
}

SystemTrayNotificationHandler::~SystemTrayNotificationHandler() {
  MVPN_COUNT_DTOR(SystemTrayNotificationHandler);
}

void SystemTrayNotificationHandler::notify(const QString& title,
                                           const QString& message,
                                           int timerSec) {
  QIcon icon(Constants::LOGO_URL);

  SystemTrayHandler::instance()->showMessage(title, message, icon,
                                             timerSec * 1000);
}
