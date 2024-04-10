/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskfunction.h"

#include "utils/leakdetector/leakdetector.h"

TaskFunction::TaskFunction(std::function<void()>&& callback,
                           Task::DeletePolicy deletePolicy)
    : Task("TaskFunction"),
      m_callback(std::move(callback)),
      m_deletePolicy(deletePolicy) {
  MZ_COUNT_CTOR(TaskFunction);
}

TaskFunction::~TaskFunction() { MZ_COUNT_DTOR(TaskFunction); }

void TaskFunction::run() {
  m_callback();
  emit completed();
}
