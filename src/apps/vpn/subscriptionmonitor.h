/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SUBSCRIPTIONMONITOR_H
#define SUBSCRIPTIONMONITOR_H

#include <QObject>

/**
 * @brief this class monitors the Controller and ConnectionHealth to check for
 * subscription status after the controller enters the No Signal state
 */
class SubscriptionMonitor final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SubscriptionMonitor)

 public:
  static SubscriptionMonitor* instance();

  ~SubscriptionMonitor();

 private:
  explicit SubscriptionMonitor(QObject* parent);
};

#endif  // SUBSCRIPTIONMONITOR_H
