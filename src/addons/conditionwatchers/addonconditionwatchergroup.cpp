/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchergroup.h"

#include "utils/leakdetector/leakdetector.h"

AddonConditionWatcherGroup::AddonConditionWatcherGroup(
    QObject* parent, const QList<AddonConditionWatcher*>& group)
    : AddonConditionWatcher(parent), m_group(group) {
  MZ_COUNT_CTOR(AddonConditionWatcherGroup);

  m_currentStatus = conditionApplied();

  for (AddonConditionWatcher* watcher : m_group) {
    connect(watcher, &AddonConditionWatcher::conditionChanged, this,
            [this](bool) {
              bool newStatus = conditionApplied();
              if (newStatus != m_currentStatus) {
                m_currentStatus = newStatus;
                emit conditionChanged(m_currentStatus);
              }
            });
  }
}

AddonConditionWatcherGroup::~AddonConditionWatcherGroup() {
  MZ_COUNT_DTOR(AddonConditionWatcherGroup);
}

bool AddonConditionWatcherGroup::conditionApplied() const {
  for (AddonConditionWatcher* watcher : m_group) {
    if (!watcher->conditionApplied()) {
      return false;
    }
  }
  return true;
}
