/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidstartatbootwatcher.h"
#include "logger.h"
#include "androidcontroller.h"
#include "platforms/android/androidutils.h"

#include <jni.h>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>

namespace {
Logger logger(LOG_ANDROID, "AndroidStartAtBootWatcher");
}

AndroidStartAtBootWatcher::AndroidStartAtBootWatcher(bool startAtBoot)
{
    startAtBootChanged(startAtBoot);
}

void AndroidStartAtBootWatcher::startAtBootChanged(bool startAtBoot)
{
    logger.log() << "StartAtBoot changed:" << startAtBoot;
    AndroidController::instance()->enableStartAtBoot(startAtBoot);
}
