/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertimestart.h"
#include "leakdetector.h"
#include "mfbt/checkedint.h"
#include "settingsholder.h"

#include <QDateTime>

AddonConditionWatcherTimeStart::AddonConditionWatcherTimeStart(QObject* parent,
                                                               qint64 time)
    : AddonConditionWatcher(parent), m_time(time) {
  MVPN_COUNT_CTOR(AddonConditionWatcherTimeStart);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() {
    if (maybeStartTimer()) {
      emit conditionChanged(true);
    }
  });

  maybeStartTimer();
}

AddonConditionWatcherTimeStart::~AddonConditionWatcherTimeStart() {
  MVPN_COUNT_DTOR(AddonConditionWatcherTimeStart);
}

bool AddonConditionWatcherTimeStart::conditionApplied() const {
  return !m_timer.isActive();
}

bool AddonConditionWatcherTimeStart::maybeStartTimer() {
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
