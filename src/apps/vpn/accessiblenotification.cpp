/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "accessiblenotification.h"

#include "logger.h"

namespace {
Logger logger("AccessibleNotification");
}

// static
AccessibleNotification* AccessibleNotification::instance() {
  static AccessibleNotification s_instance;
  return &s_instance;
}

Q_INVOKABLE void AccessibleNotification::notify(QQuickItem* sourceItem,
                                                const QString& notification) {
  logger.debug() << notification;

  QAccessibleValueChangeEvent event(sourceItem, QAccessible::ValueChanged);
  event.setValue(notification);
  QAccessible::updateAccessibility(&event);
}
