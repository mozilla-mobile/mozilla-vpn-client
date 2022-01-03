/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKACCOUNT_H
#define TASKACCOUNT_H

#include "task.h"

#include <QObject>

class TaskAccount final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAccount)

 public:
  TaskAccount();
  ~TaskAccount();

  void run() override;
};

#endif  // TASKACCOUNT_H
