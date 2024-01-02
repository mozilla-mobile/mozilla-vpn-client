/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNRELEASEMONITOR_H
#define VPNRELEASEMONITOR_H

#include <QQmlEngine>

#include "releasemonitor.h"
#include "mozillavpn.h"

struct VPNReleaseMonitor {
  Q_GADGET
  QML_FOREIGN(ReleaseMonitor)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ReleaseMonitor* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->releaseMonitor();
  }
};

#endif  // VPNRELEASEMONITOR_H
