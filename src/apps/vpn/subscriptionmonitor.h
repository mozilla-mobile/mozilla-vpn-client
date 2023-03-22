/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SUBSCRIPTIONMONITOR_H
#define SUBSCRIPTIONMONITOR_H

#include <QObject>

/**
 * @brief this class watches a few setting properties to see if we need to
 * trigger a silent-server-switch
 */
class SubscriptionMonitor final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SubscriptionMonitor)

 public:
  static SubscriptionMonitor* instance();

  ~SubscriptionMonitor();

 private:
  explicit SubscriptionMonitor(QObject* parent);

  void operationCompleted();

 private:
  class TaskSubscriptionMonitor;

  bool m_operationRunning = false;
};

#endif  // SUBSCRIPTIONMONITOR_H
