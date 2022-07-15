/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTRIGGERTIMESECS_H
#define ADDONCONDITIONWATCHERTRIGGERTIMESECS_H

#include "addonconditionwatcher.h"

#include <QTimer>

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

 private:
  QTimer m_timer;
};

#endif  // ADDONCONDITIONWATCHERTRIGGERTIMESECS_H
