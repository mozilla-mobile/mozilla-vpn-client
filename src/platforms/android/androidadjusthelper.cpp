/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidadjusthelper.h"

#include <QAndroidJniObject>
#include <QString>

void AndroidAdjustHelper::trackEvent(const QString& event) {
  QAndroidJniObject javaMessage = QAndroidJniObject::fromString(event);
  QAndroidJniObject::callStaticMethod<void>("org/mozilla/firefox/vpn/qt/VPNAdjustHelper",
                                            "trackEvent",
                                            "(Ljava/lang/String;)V",
                                            javaMessage.object<jstring>());
}