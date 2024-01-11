/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKCAPTIVEPORTALLOOKUP_H
#define TASKCAPTIVEPORTALLOOKUP_H

#include <QObject>

#include "errorhandler.h"
#include "taskscheduler/task.h"

class TaskCaptivePortalLookup final : public Task {
  Q_DISABLE_COPY_MOVE(TaskCaptivePortalLookup)

 public:
  explicit TaskCaptivePortalLookup(
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskCaptivePortalLookup();

  void run() override;

 private:
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKCAPTIVEPORTALLOOKUP_H
