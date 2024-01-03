/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNANDROIDUTILS_H
#define VPNANDROIDUTILS_H

#include <QQmlEngine>

#include "platforms/android/androidutils.h"

struct VPNAndroidUtils {
  Q_GADGET
  QML_FOREIGN(AndroidUtils)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static AndroidUtils* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return AndroidUtils::instance();
  }
};

#endif  // VPNANDROIDUTILS_H
