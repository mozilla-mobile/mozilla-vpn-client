/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTIMEEND_H
#define ADDONCONDITIONWATCHERTIMEEND_H

#include <QTimer>

#include "addonconditionwatchertime.h"

class AddonConditionWatcherTimeEnd final : public AddonConditionWatcherTime {
 public:
  AddonConditionWatcherTimeEnd(QObject* parent, qint64 time)
      : AddonConditionWatcherTime(parent, time, false) {}
  ~AddonConditionWatcherTimeEnd() = default;
};

#endif  // ADDONCONDITIONWATCHERTIMEEND_H
