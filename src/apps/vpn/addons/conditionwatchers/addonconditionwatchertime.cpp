/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertime.h"

#include <QDateTime>

#include "leakdetector.h"
#include "mfbt/checkedint.h"
#include "settingsholder.h"

AddonConditionWatcherTime::AddonConditionWatcherTime(QObject* parent,
                                                     qint64 time, bool isStart)
    : AddonConditionWatcher(parent), m_time(time), m_isStart(isStart) {
  MZ_COUNT_CTOR(AddonConditionWatcherTime);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() {
    if (maybeStartTimer()) {
      emit conditionChanged(m_isStart);
    }
  });

  maybeStartTimer();
}

AddonConditionWatcherTime::~AddonConditionWatcherTime() {
  MZ_COUNT_DTOR(AddonConditionWatcherTime);
}

bool AddonConditionWatcherTime::conditionApplied() const {
  return m_isStart != m_timer.isActive();
}

bool AddonConditionWatcherTime::maybeStartTimer() {
  qint64 currentTime = QDateTime::currentSecsSinceEpoch();
  if (m_time <= currentTime) {
    return true;
  }

  CheckedInt<int> value(static_cast<int>(m_time - currentTime));
  value *= 1000;

  m_timer.start(value.isValid() ? value.value()
                                : std::numeric_limits<int>::max());

  return false;
}
