/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "tasks/release/taskrelease.h"
#include "taskscheduler.h"
#include "timersingleshot.h"
#include "update/updater.h"

namespace {
Logger logger(LOG_MAIN, "ReleaseMonitor");
}

ReleaseMonitor::ReleaseMonitor() {
  MVPN_COUNT_CTOR(ReleaseMonitor);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &ReleaseMonitor::runSoon);
}

ReleaseMonitor::~ReleaseMonitor() { MVPN_COUNT_DTOR(ReleaseMonitor); }

void ReleaseMonitor::runSoon() {
  logger.debug() << "Scheduling a release-check task";

  TimerSingleShot::create(this, 0, [this] {
    TaskRelease* task = new TaskRelease(TaskRelease::Check);

    connect(task, &TaskRelease::updaterFailure, []() {
      // This cannot happen for release checks.
      Q_ASSERT(false);
    });

    connect(task, &TaskRelease::updateRequired, this,
            &ReleaseMonitor::updateRequired);
    connect(task, &TaskRelease::updateRecommended, this,
            &ReleaseMonitor::updateRecommended);
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
  m_timer.start(Constants::releaseMonitorMsec());
}

void ReleaseMonitor::updateRequired() {
  logger.warning() << "update required";
  MozillaVPN::instance().setUpdateRecommended(false);
  MozillaVPN::instance().controller()->updateRequired();
}

void ReleaseMonitor::updateRecommended() {
  logger.debug() << "Update recommended";
  MozillaVPN::instance().setUpdateRecommended(true);
}

void ReleaseMonitor::updateSoon() {
  logger.debug() << "Scheduling a release-update task";

  TimerSingleShot::create(this, 0, [] {
    TaskRelease* task = new TaskRelease(TaskRelease::Update);
    connect(task, &TaskRelease::updaterFailure, []() {
      logger.warning() << "No updater supported for this platform. Fallback";

      auto& vpn = MozillaVPN::instance();
      vpn.openLink(MozillaVPN::LinkUpdate);
      vpn.setUpdating(false);
    });

    // The updater, in download mode, is not destroyed. So, if this happens,
    // probably something went wrong.
    connect(task, &Task::completed, [] {
      auto& vpn = MozillaVPN::instance();
      vpn.setUpdating(false);
    });

    TaskScheduler::scheduleTask(task);
  });
}
