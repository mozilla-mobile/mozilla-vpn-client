/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskrelease.h"
#include "leakdetector.h"
#include "logger.h"
#include "update/updater.h"

namespace {
Logger logger(LOG_MAIN, "TaskRelease");
}

TaskRelease::TaskRelease(Op op) : Task("TaskRelease"), m_op(op) {
  MVPN_COUNT_CTOR(TaskRelease);
}

TaskRelease::~TaskRelease() { MVPN_COUNT_DTOR(TaskRelease); }

void TaskRelease::run() {
  logger.debug() << "Release check started";

  Updater* updater = Updater::create(this, m_op == Update);
  if (!updater) {
    emit updaterFailure();
    emit completed();
    return;
  }

  connect(updater, &Updater::updateRequired, this,
          &TaskRelease::updateRequired);
  connect(updater, &Updater::updateRecommended, this,
          &TaskRelease::updateRecommended);
  connect(updater, &Updater::updateRequired, this,
          &TaskRelease::updateRequiredOrRecommended);
  connect(updater, &Updater::updateRecommended, this,
          &TaskRelease::updateRequiredOrRecommended);
  connect(updater, &QObject::destroyed, this, [this, updater]() {
    if (!updater->recommendedOrRequired()) {
      emit updateNotAvailable();
    }
    emit completed();
  });

  updater->start(this);
}
