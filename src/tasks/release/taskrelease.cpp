/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskrelease.h"

#include "logging/logger.h"
#include "update/updater.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("TaskRelease");
}

TaskRelease::TaskRelease(
    Op op, ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskRelease"),
      m_op(op),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskRelease);
}

TaskRelease::~TaskRelease() { MZ_COUNT_DTOR(TaskRelease); }

void TaskRelease::run() {
  logger.debug() << "Release check started";

  Updater* updater =
      Updater::create(this, m_op == Update, m_errorPropagationPolicy);
  Q_ASSERT(updater);

  connect(updater, &Updater::updateRequired, this,
          &TaskRelease::updateRequired);
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
