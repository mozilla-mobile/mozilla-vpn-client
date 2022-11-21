/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertriggertimesecs.h"
#include "leakdetector.h"
#include "mfbt/checkedint.h"
#include "settingsholder.h"

#include <QDateTime>

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
    : AddonConditionWatcher(parent) {
  MVPN_COUNT_CTOR(AddonConditionWatcherTriggerTimeSecs);

  QDateTime now = QDateTime::currentDateTime();
  QDateTime installation = SettingsHolder::instance()->installationTime();

  // Note: triggerTimeSecs is seconds!
  CheckedInt<int> secs =
      static_cast<int>(triggerTimeSecs - installation.secsTo(now));
  if (secs.value() > 0) {
    m_timer.setSingleShot(true);
    secs *= 1000;
    m_timer.start(secs.value());

    connect(&m_timer, &QTimer::timeout, this,
            [this]() { emit conditionChanged(true); });
  }
}

AddonConditionWatcherTriggerTimeSecs::~AddonConditionWatcherTriggerTimeSecs() {
  MVPN_COUNT_DTOR(AddonConditionWatcherTriggerTimeSecs);
}

bool AddonConditionWatcherTriggerTimeSecs::conditionApplied() const {
  return !m_timer.isActive();
}
