/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNANDROIDCOMMONS_H
#define VPNANDROIDCOMMONS_H

#include <QQmlEngine>

#include "platforms/android/androidcommons.h"

class VPNAndroidCommons : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

 public:
  Q_INVOKABLE int getAndroidApiLevel() {
    return AndroidCommons::getSDKVersion();
  }
};

#endif  // VPNANDROIDCOMMONS_H
