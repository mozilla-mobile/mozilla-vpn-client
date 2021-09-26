/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskfunction.h"
#include "leakdetector.h"

TaskFunction::TaskFunction(std::function<void(Core*)>&& callback)
    : Task("TaskFunction"), m_callback(std::move(callback)) {
  MVPN_COUNT_CTOR(TaskFunction);
}

TaskFunction::~TaskFunction() { MVPN_COUNT_DTOR(TaskFunction); }

void TaskFunction::run(Core* core) {
  m_callback(core);
  emit completed();
}
