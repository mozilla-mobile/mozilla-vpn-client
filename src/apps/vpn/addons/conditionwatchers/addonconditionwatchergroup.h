/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONCONDITIONWATCHERGROUP_H
#define ADDONCONDITIONWATCHERGROUP_H

#include "addonconditionwatcher.h"

class AddonConditionWatcherGroup final : public AddonConditionWatcher {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonConditionWatcherGroup)

 public:
  AddonConditionWatcherGroup(QObject* parent,
                             const QList<AddonConditionWatcher*>& group);
  ~AddonConditionWatcherGroup();

  bool conditionApplied() const override;

 private:
  const QList<AddonConditionWatcher*> m_group;
  bool m_currentStatus = false;
};

#endif  // ADDONCONDITIONWATCHERGROUP_H
