/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertriggertimesecs.h"

#include <QDateTime>

#include "leakdetector.h"
#include "mfbt/checkedint.h"
#include "settingsholder.h"

// static
AddonConditionWatcher* AddonConditionWatcherTriggerTimeSecs::maybeCreate(
    QObject* parent, qint64 triggerTimeSecs) {
  if (triggerTimeSecs == 0) {
    return nullptr;
  }

  return new AddonConditionWatcherTriggerTimeSecs(parent, triggerTimeSecs);
}

AddonConditionWatcherTriggerTimeSecs::AddonConditionWatcherTriggerTimeSecs(
    QObject* parent, qint64 triggerTimeSecs)
    : AddonConditionWatcher(parent), m_triggerTimeSecs(triggerTimeSecs) {
  MZ_COUNT_CTOR(AddonConditionWatcherTriggerTimeSecs);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() {
    if (maybeStartTimer()) {
      emit conditionChanged(true);
    }
  });

  maybeStartTimer();
}

AddonConditionWatcherTriggerTimeSecs::~AddonConditionWatcherTriggerTimeSecs() {
  MZ_COUNT_DTOR(AddonConditionWatcherTriggerTimeSecs);
}

bool AddonConditionWatcherTriggerTimeSecs::conditionApplied() const {
  return !m_timer.isActive();
}

bool AddonConditionWatcherTriggerTimeSecs::maybeStartTimer() {
  QDateTime now = QDateTime::currentDateTime();
  QDateTime expire =
      SettingsHolder::instance()->installationTime().addSecs(m_triggerTimeSecs);

  if (expire <= now) {
    return true;
  }

  m_timer.setSingleShot(true);
  m_timer.start(now.msecsTo(expire));

  return false;
}
