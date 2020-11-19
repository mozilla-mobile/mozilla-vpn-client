/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/android/androidnotificationhandler.h"

AndroidNotificationHandler::AndroidNotificationHandler(QObject *parent)
    : NotificationHandler(parent)
{}

void AndroidNotificationHandler::notify(const QString &title, const QString &message, int timerSec)
{
    // TODO
    Q_UNUSED(title);
    Q_UNUSED(message);
    Q_UNUSED(timerSec);
}
