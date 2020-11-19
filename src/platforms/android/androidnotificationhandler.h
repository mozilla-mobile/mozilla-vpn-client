/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDNOTIFICATIONHANDLER_H
#define ANDROIDNOTIFICATIONHANDLER_H

#include "notificationhandler.h"

class AndroidNotificationHandler final : public NotificationHandler
{
public:
    AndroidNotificationHandler(QObject *parent);

protected:
    void notify(const QString &title, const QString &message, int timerSec) override;
};

#endif // ANDROIDNOTIFICATIONHANDLER_H
