/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSNOTIFICATIONHANDLER_H
#define IOSNOTIFICATIONHANDLER_H

#include "notificationhandler.h"

class IOSNotificationHandler final : public NotificationHandler
{
public:
    IOSNotificationHandler(QObject *parent);

protected:
    void notify(const QString &title, const QString &message) override;
};

#endif // IOSNOTIFICATIONHANDLER_H
