/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosstartatbootwatcher.h"
#include "macosutils.h"

#include <QDebug>

MacOSStartAtBootWatcher::MacOSStartAtBootWatcher(bool startAtBoot)
{
    qDebug() << "StartAtBoot watcher";
    MacOSUtils::enableLoginItem(startAtBoot);
}

void MacOSStartAtBootWatcher::startAtBootChanged(bool startAtBoot)
{
    qDebug() << "StartAtBoot changed:" << startAtBoot;
    MacOSUtils::enableLoginItem(startAtBoot);
}
