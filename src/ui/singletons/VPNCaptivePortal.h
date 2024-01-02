/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCAPTIVEPORTAL_H
#define VPNCAPTIVEPORTAL_H

#include <QQmlEngine>

#include "captiveportal/captiveportaldetection.h"
#include "mozillavpn.h"

struct VPNCaptivePortal {
  Q_GADGET
  QML_FOREIGN(CaptivePortalDetection)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static CaptivePortalDetection* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->captivePortalDetection();
  }
};

#endif  // VPNCAPTIVEPORTAL_H
