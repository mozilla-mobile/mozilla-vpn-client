
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENTRY_H
#define TASKSENTRY_H

#include <QByteArray>
#include <QObject>

#include "task.h"

class TaskSentry final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSentry)

 public:
  TaskSentry(const QByteArray& envelope);
  ~TaskSentry();

  void run() override;

 private:
  QByteArray m_envelope;
};

#endif  // TASKSENTRY_H