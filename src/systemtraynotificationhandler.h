/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTEMTRAYNOTIFICATIONHANDLER_H
#define SYSTEMTRAYNOTIFICATIONHANDLER_H

#include "notificationhandler.h"

class SystemTrayNotificationHandler final : public NotificationHandler {
 public:
  SystemTrayNotificationHandler(QObject* parent);
  ~SystemTrayNotificationHandler();

 protected:
  void notify(const QString& title, const QString& message,
              int timerSec) override;
};

#endif  // SYSTEMTRAYNOTIFICATIONHANDLER_H
