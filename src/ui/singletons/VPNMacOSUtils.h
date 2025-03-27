/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNMACOSUTILS_H
#define VPNMACOSUTILS_H

#include <QQmlEngine>

#include "platforms/macos/macosutils.h"

struct VPNMacOSUtils {
  Q_GADGET
  QML_FOREIGN(MacOSUtils)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static MacOSUtils* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MacOSUtils::instance();
  }
};

#endif  // VPNMACOSUTILS_H
