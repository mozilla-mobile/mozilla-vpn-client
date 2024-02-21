/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidgleanbridge.h"

#include <jni.h>

#include <QJniEnvironment>
#include <QJniObject>

#include "context/androidconstants.h"
#include "settings/settingsholder.h"
#include "utilities/androidutils.h"

void AndroidGleanBridge::initializeGlean(bool isTelemetryEnabled,
                                         const QString& channel) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  QString gleanDebugTag = settingsHolder->gleanDebugTagActive()
                              ? settingsHolder->gleanDebugTag()
                              : "";

  AndroidUtils::runOnAndroidThreadSync(
      [isTelemetryEnabled, channel, gleanDebugTag]() {
        QJniObject::callStaticMethod<void>(
            AndroidConstants::COMMON_UTILS_CLASS, "initializeGlean",
            "(Landroid/content/Context;ZLjava/lang/String;Ljava/lang/String;)V",
            AndroidUtils::getActivity().object(), (jboolean)isTelemetryEnabled,
            QJniObject::fromString(channel).object(),
            QJniObject::fromString(gleanDebugTag).object());
      });
}
