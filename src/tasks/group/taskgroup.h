/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <QList>
#include <QObject>

#include "task.h"

class TaskGroup final : public Task {
  Q_DISABLE_COPY_MOVE(TaskGroup)

 public:
  TaskGroup(const QList<Task*>& list);
  ~TaskGroup();

  void run() override;

  void cancel() override;
  DeletePolicy deletePolicy() const override;

 private:
  void maybeComplete();

 private:
  QList<Task*> m_tasks;
};

#endif  // TASKGROUP_H
