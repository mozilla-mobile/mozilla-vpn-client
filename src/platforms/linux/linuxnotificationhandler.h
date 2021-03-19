/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXNOTIFICATIONHANDLER_H
#define LINUXNOTIFICATIONHANDLER_H

#include "notificationhandler.h"

#include <QObject>

class LinuxNotificationHandler final : public NotificationHandler {
  Q_DISABLE_COPY_MOVE(LinuxNotificationHandler)

 public:
  LinuxNotificationHandler(QObject* parent);
  ~LinuxNotificationHandler();

 protected:
  void notify(const QString& title, const QString& message,
              int timerSec) override;
};

#endif  // LINUXNOTIFICATIONHANDLER_H
