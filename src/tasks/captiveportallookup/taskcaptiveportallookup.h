/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKCAPTIVEPORTALLOOKUP_H
#define TASKCAPTIVEPORTALLOOKUP_H

#include "errorhandler.h"
#include "task.h"

#include <QObject>

class TaskCaptivePortalLookup final : public Task {
  Q_DISABLE_COPY_MOVE(TaskCaptivePortalLookup)

 public:
  explicit TaskCaptivePortalLookup(
      ErrorHandler::ErrorPropagation errorPropagation);
  ~TaskCaptivePortalLookup();

  void run() override;

 private:
  ErrorHandler::ErrorPropagation m_errorPropagation =
      ErrorHandler::NoErrorPropagation;
};

#endif  // TASKCAPTIVEPORTALLOOKUP_H
