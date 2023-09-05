/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ACCESSIBLENOTIFICATION_H
#define ACCESSIBLENOTIFICATION_H

#include <QAccessible>
#include <QObject>
#include <QQuickItem>
#include <QString>

// Singleton that notifies accessibility clients, like screen readers, using
// a string. This can be used to notify the clients of any state change, and
// a screen reader will read the provided string.
class AccessibleNotification : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AccessibleNotification)

  AccessibleNotification() = default;
  virtual ~AccessibleNotification() = default;

 public:
  static AccessibleNotification* instance();

  // Notify accessibility client of the notification string.
  // sourceItem: source of the change. It should have an Accessible
  // attached property (if in QML) or QAccessibleInterface.
  Q_INVOKABLE void notify(QQuickItem* sourceItem, const QString& notification);
};

#endif  // ACCESSIBLENOTIFICATION_H