
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENTRYCONFIG_H
#define TASKSENTRYCONFIG_H

#include <QByteArray>
#include <QObject>

#include "task.h"

class TaskSentryConfig final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSentryConfig)

 public:
  TaskSentryConfig();
  ~TaskSentryConfig();

  void run() override;

 private:
  static bool handleNetworkResponse(const QByteArray& data);
};

#endif  // TASKSENTRYCONFIG_H