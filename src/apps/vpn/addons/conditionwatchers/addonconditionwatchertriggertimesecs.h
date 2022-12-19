/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTRIGGERTIMESECS_H
#define ADDONCONDITIONWATCHERTRIGGERTIMESECS_H

#include <QTimer>

#include "addonconditionwatcher.h"

class AddonConditionWatcherTriggerTimeSecs final
    : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherTriggerTimeSecs)

 public:
  ~AddonConditionWatcherTriggerTimeSecs();

  static AddonConditionWatcher* maybeCreate(QObject* parent, qint64 time);

  bool conditionApplied() const override;

 private:
  AddonConditionWatcherTriggerTimeSecs(QObject* parent, qint64 time);

  // Return true if the timer does not need to run because the condition
  // matches already.
  bool maybeStartTimer();

 private:
  qint64 m_triggerTimeSecs = 0;
  QTimer m_timer;
};

#endif  // ADDONCONDITIONWATCHERTRIGGERTIMESECS_H
