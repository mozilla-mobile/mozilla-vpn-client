/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSERVERSELECT_H
#define TASKSERVERSELECT_H

#include "errorhandler.h"
#include "task.h"

#include <QObject>

class TaskServerSelect final : public Task {
  Q_DISABLE_COPY_MOVE(TaskServerSelect)

 public:
  explicit TaskServerSelect(
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskServerSelect();

  void run() override;

 private:
  QStringList processData(const QByteArray& data);
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKSERVERSELECT_H
