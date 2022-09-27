/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTIMEEND_H
#define ADDONCONDITIONWATCHERTIMEEND_H

#include "addonconditionwatcher.h"

#include <QTimer>

class AddonConditionWatcherTimeEnd final : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherTimeEnd)

 public:
  AddonConditionWatcherTimeEnd(QObject* parent, qint64 time);
  ~AddonConditionWatcherTimeEnd();

  bool conditionApplied() const override;

 private:
  QTimer m_timer;
};

#endif  // ADDONCONDITIONWATCHERTIMEEND_H
