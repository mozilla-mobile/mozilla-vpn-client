/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKTOKEN_H
#define TASKTOKEN_H

#include <QObject>

#include "errorhandler.h"
#include "task.h"

class TaskToken final : public Task {
  Q_DISABLE_COPY_MOVE(TaskToken)

 public:
  explicit TaskToken(
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskToken();

  void run() override;

 private:
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKTOKEN_H