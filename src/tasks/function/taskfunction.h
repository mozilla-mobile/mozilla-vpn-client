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
  TaskFunction(std::function<void()>&& callback, bool deletable = true);
  ~TaskFunction();

  void run() override;

  bool deletable() const override { return m_deletable; }

 private:
  std::function<void()> m_callback;
  bool m_deletable = true;
};

#endif  // TASKFUNCTION_H
