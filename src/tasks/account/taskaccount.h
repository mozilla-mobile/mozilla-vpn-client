/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKACCOUNT_H
#define TASKACCOUNT_H

#include <QObject>

#include "errorhandler.h"
#include "taskscheduler/task.h"

class TaskAccount final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAccount)

 public:
  explicit TaskAccount(
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskAccount();

  void run() override;

 private:
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKACCOUNT_H
