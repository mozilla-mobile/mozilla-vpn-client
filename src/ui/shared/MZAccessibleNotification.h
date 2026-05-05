/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "accessiblenotification.h"

struct MZAccessibleNotification {
  Q_GADGET
  QML_FOREIGN(AccessibleNotification)
  QML_NAMED_ELEMENT(MZAccessibleNotification)
  QML_SINGLETON

 public:
  static AccessibleNotification* create(QQmlEngine*, QJSEngine*) {
    return AccessibleNotification::instance();
  }
};
