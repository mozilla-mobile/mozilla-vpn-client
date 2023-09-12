/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTIME_H
#define ADDONCONDITIONWATCHERTIME_H

#include <QTimer>

#include "addonconditionwatcher.h"

class AddonConditionWatcherTime : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherTime)

 public:
  AddonConditionWatcherTime(QObject* parent, qint64 time, bool isStart);
  virtual ~AddonConditionWatcherTime();

  bool conditionApplied() const override;

 private:
  // Return true if the timer does not need to run because the condition
  // matches already.
  bool maybeStartTimer();

 private:
  qint64 m_time = 0;
  bool m_isStart = false;
  QTimer m_timer;
};

#endif  // ADDONCONDITIONWATCHERTIMESTART_H
