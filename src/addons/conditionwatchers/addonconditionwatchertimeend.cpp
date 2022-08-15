/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertimeend.h"
#include "leakdetector.h"
#include "settingsholder.h"

#include <QDateTime>

AddonConditionWatcherTimeEnd::AddonConditionWatcherTimeEnd(QObject* parent,
                                                           qint64 time)
    : AddonConditionWatcher(parent) {
  MVPN_COUNT_CTOR(AddonConditionWatcherTimeEnd);

  qint64 currentTime = QDateTime::currentSecsSinceEpoch();
  if (time > currentTime) {
    m_timer.setSingleShot(true);
    m_timer.start((time - currentTime) * 1000);

    connect(&m_timer, &QTimer::timeout, this,
            [this]() { emit conditionChanged(false); });
  }
}

AddonConditionWatcherTimeEnd::~AddonConditionWatcherTimeEnd() {
  MVPN_COUNT_DTOR(AddonConditionWatcherTimeEnd);
}

bool AddonConditionWatcherTimeEnd::conditionApplied() const {
  return m_timer.isActive();
}
