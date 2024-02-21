/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKHEARTBEAT_H
#define TASKHEARTBEAT_H

#include <QObject>

#include "taskscheduler/task.h"

class TaskHeartbeat final : public Task {
  Q_DISABLE_COPY_MOVE(TaskHeartbeat)

 public:
  TaskHeartbeat();
  ~TaskHeartbeat();

  void run() override;
};

#endif  // TASKHEARTBEAT_H
