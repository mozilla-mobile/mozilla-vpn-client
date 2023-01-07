/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKGETLOCATION_H
#define TASKGETLOCATION_H

#include "errorhandler.h"
#include "task.h"

class TaskGetLocation final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskGetLocation)

 public:
  explicit TaskGetLocation(
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskGetLocation();

  void run() override;

 private:
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKGETLOCATION_H
