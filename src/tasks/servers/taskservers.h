/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSERVERS_H
#define TASKSERVERS_H

#include "errorhandler.h"
#include "task.h"

#include <QObject>

class TaskServers final : public Task {
  Q_DISABLE_COPY_MOVE(TaskServers)

 public:
  explicit TaskServers(ErrorHandler::ErrorPropagation errorPropagation);
  ~TaskServers();

  void run() override;

 private:
  ErrorHandler::ErrorPropagation m_errorPropagation =
      ErrorHandler::NoErrorPropagation;
};

#endif  // TASKSERVERS_H
