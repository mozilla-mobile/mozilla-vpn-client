/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetfeaturelistworker.h"

#include "leakdetector.h"
#include "settingsholder.h"
#include "taskgetfeaturelist.h"
#include "taskscheduler.h"

TaskGetFeatureListWorker::TaskGetFeatureListWorker(QObject* parent)
    : QObject(parent) {
  MZ_COUNT_CTOR(TaskGetFeatureListWorker);
}

TaskGetFeatureListWorker::~TaskGetFeatureListWorker() {
  MZ_COUNT_DTOR(TaskGetFeatureListWorker);
}

void TaskGetFeatureListWorker::start(std::chrono::milliseconds interval) {
  scheduleTask();

  // The `token` setting is the setting that contains the JWT token for authed
  // users. When it changes it means the user has either logged in or out.
  //
  // Experiments are tied to users, so when the user changes or signs out,
  // the active experiments may also change.
  connect(SettingsHolder::instance(), &SettingsHolder::tokenChanged, this,
          &TaskGetFeatureListWorker::scheduleTask);
  connect(&m_timer, &QTimer::timeout, this,
          &TaskGetFeatureListWorker::scheduleTask);

  m_timer.start(interval);
}

void TaskGetFeatureListWorker::scheduleTask() {
  TaskScheduler::scheduleTask(new TaskGetFeatureList());
}
