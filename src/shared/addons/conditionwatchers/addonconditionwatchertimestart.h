/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERTIMESTART_H
#define ADDONCONDITIONWATCHERTIMESTART_H

#include "addonconditionwatchertime.h"

class AddonConditionWatcherTimeStart final : public AddonConditionWatcherTime {
 public:
  AddonConditionWatcherTimeStart(QObject* parent, qint64 time)
      : AddonConditionWatcherTime(parent, time, true) {}
  ~AddonConditionWatcherTimeStart() = default;
};

#endif  // ADDONCONDITIONWATCHERTIMESTART_H
