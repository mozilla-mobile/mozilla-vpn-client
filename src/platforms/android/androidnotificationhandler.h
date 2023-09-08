/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDNOTIFICATIONHANDLER_H
#define ANDROIDNOTIFICATIONHANDLER_H

#include <QObject>

#include "notificationhandler.h"

class AndroidNotificationHandler final : public NotificationHandler {
  Q_DISABLE_COPY_MOVE(AndroidNotificationHandler)

 public:
  AndroidNotificationHandler(QObject* parent);
  ~AndroidNotificationHandler();

 protected:
  void notify(Message type, const QString& title, const QString& message,
              int timerMsec) override;

 private:
  void initialize() override;
};

#endif  // ANDROIDNOTIFICATIONHANDLER_H
