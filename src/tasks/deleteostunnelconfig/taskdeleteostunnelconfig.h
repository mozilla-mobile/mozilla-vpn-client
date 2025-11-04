/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKDELETEOSTUNNELCONFIG_H
#define TASKDELETEOSTUNNELCONFIG_H

#include <QObject>

#include "task.h"

// The purpose of this task is to remove the OSs tunnel config.
class TaskDeleteOSTunnelConfig final : public Task {
  Q_DISABLE_COPY_MOVE(TaskDeleteOSTunnelConfig)

 public:
  explicit TaskDeleteOSTunnelConfig();

  ~TaskDeleteOSTunnelConfig();

  void run() override;

  virtual DeletePolicy deletePolicy() const override { return NonDeletable; }

 private:
};

#endif  // TASKDELETEOSTUNNELCONFIG_H
