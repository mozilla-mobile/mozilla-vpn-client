/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKFUNCTION_H
#define TASKFUNCTION_H

#include "task.h"

#include <functional>
#include <QObject>

class TaskFunction final : public Task {
  Q_DISABLE_COPY_MOVE(TaskFunction)

 public:
  TaskFunction(std::function<void()>&& callback, DeletePolicy = Deletable);
  ~TaskFunction();

  void run() override;

  DeletePolicy deletePolicy() const override { return m_deletePolicy; }

 private:
  std::function<void()> m_callback;
  DeletePolicy m_deletePolicy = Deletable;
};

#endif  // TASKFUNCTION_H
