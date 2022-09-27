/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertimestart.h"
#include "leakdetector.h"
#include "settingsholder.h"

#include <QDateTime>

AddonConditionWatcherTimeStart::AddonConditionWatcherTimeStart(QObject* parent,
                                                               qint64 time)
    : AddonConditionWatcher(parent) {
  MVPN_COUNT_CTOR(AddonConditionWatcherTimeStart);

  qint64 currentTime = QDateTime::currentSecsSinceEpoch();
  if (time > currentTime) {
    m_timer.setSingleShot(true);
    m_timer.start((time - currentTime) * 1000);

    connect(&m_timer, &QTimer::timeout, this,
            [this]() { emit conditionChanged(true); });
  }
}

AddonConditionWatcherTimeStart::~AddonConditionWatcherTimeStart() {
  MVPN_COUNT_DTOR(AddonConditionWatcherTimeStart);
}

bool AddonConditionWatcherTimeStart::conditionApplied() const {
  return !m_timer.isActive();
}
