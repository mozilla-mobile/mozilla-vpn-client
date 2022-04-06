/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSERVERVERIFY_H
#define TASKSERVERVERIFY_H

#include "task.h"
#include "models/server.h"

#include <QObject>
#include <QTimer>

class PingSender;

class TaskServerVerify final : public Task {
  Q_DISABLE_COPY_MOVE(TaskServerVerify)

 public:
  TaskServerVerify(const QString& serverPublicKey);
  ~TaskServerVerify();

  void run() override;

 private:
  Server m_server;

  int m_pingAttempts = 0;
  int m_pingReceived = 0;
  PingSender* m_pingSender = nullptr;
  QTimer m_pingTimer;
};

#endif  // TASKSERVERVERIFY_H
