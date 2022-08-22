/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTIMESTART_H
#define ADDONCONDITIONWATCHERTIMESTART_H

#include "addonconditionwatcher.h"

#include <QTimer>

class AddonConditionWatcherTimeStart final : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherTimeStart)

 public:
  AddonConditionWatcherTimeStart(QObject* parent, qint64 time);
  ~AddonConditionWatcherTimeStart();

  bool conditionApplied() const override;

 private:
  QTimer m_timer;
};

#endif  // ADDONCONDITIONWATCHERTIMESTART_H
