/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKRELEASE_H
#define TASKRELEASE_H

#include "task.h"

#include <QObject>

class TaskRelease final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskRelease)

 public:
  enum Op {
    Check,
    Update,
  };

  explicit TaskRelease(Op op);
  ~TaskRelease();

  void run() override;

 signals:
  void updaterFailure();
  void updateRequired();
  void updateRecommended();

 private:
  Op m_op;
};

#endif  // TASKRELEASE_H
