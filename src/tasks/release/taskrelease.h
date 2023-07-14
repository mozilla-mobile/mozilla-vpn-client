/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKRELEASE_H
#define TASKRELEASE_H

#include <QObject>

#include "errorhandler.h"
#include "task.h"

class TaskRelease final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskRelease)

 public:
  enum Op {
    Check,
    Update,
  };

  TaskRelease(Op op,
              ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskRelease();

  void run() override;

  // We do not want to delete update operations, triggered by users.
  DeletePolicy deletePolicy() const override {
    return m_op == Update ? NonDeletable : Deletable;
  }

 signals:
  void updateRequired();
  void updateRequiredOrRecommended();
  void updateNotAvailable();

 private:
  Op m_op = Check;
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKRELEASE_H
