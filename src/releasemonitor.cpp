/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"

#include "appconstants.h"
#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/release/taskrelease.h"
#include "taskscheduler.h"
#include "update/updater.h"

namespace {
Logger logger("ReleaseMonitor");
}

ReleaseMonitor::ReleaseMonitor() {
  MZ_COUNT_CTOR(ReleaseMonitor);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() {
    ReleaseMonitor::runSoon(ErrorHandler::DoNotPropagateError);
  });
}

ReleaseMonitor::~ReleaseMonitor() { MZ_COUNT_DTOR(ReleaseMonitor); }

void ReleaseMonitor::runSoon(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy) {
  logger.debug() << "Scheduling a release-check task";

  QTimer::singleShot(0, this, [this, errorPropagationPolicy] {
    TaskRelease* task =
        new TaskRelease(TaskRelease::Check, errorPropagationPolicy);

    connect(task, &TaskRelease::updateRequired, this,
            &ReleaseMonitor::updateRequired);
    connect(task, &TaskRelease::updateRequiredOrRecommended, this,
            &ReleaseMonitor::updateRequiredOrRecommended);
    connect(task, &TaskRelease::updateNotAvailable, this,
            &ReleaseMonitor::updateNotAvailable);
    connect(task, &Task::completed, this, &ReleaseMonitor::releaseChecked);
    connect(task, &Task::completed, this, &ReleaseMonitor::schedule);

    TaskScheduler::scheduleTask(task);
  });
}

void ReleaseMonitor::schedule() {
  logger.debug() << "ReleaseMonitor scheduling";
  m_timer.start(AppConstants::releaseMonitorMsec());
}

void ReleaseMonitor::updateRequired() {
  logger.warning() << "update required";
  MozillaVPN::instance()->controller()->updateRequired();
}

void ReleaseMonitor::updateSoon() {
  logger.debug() << "Scheduling a release-update task";

  QTimer::singleShot(0, this, [] {
    TaskRelease* task =
        new TaskRelease(TaskRelease::Update, ErrorHandler::PropagateError);
    // The updater, in download mode, is not destroyed. So, if this happens,
    // probably something went wrong.
    QObject::connect(task, &Task::completed, task, [] {
      MozillaVPN* vpn = MozillaVPN::instance();
      Q_ASSERT(vpn);
      vpn->setUpdating(false);
    });

    TaskScheduler::scheduleTask(task);
  });
}
