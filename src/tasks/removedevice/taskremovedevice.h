/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKREMOVEDEVICE_H
#define TASKREMOVEDEVICE_H

#include <QObject>

#include "taskscheduler/task.h"

class TaskRemoveDevice final : public Task {
  Q_DISABLE_COPY_MOVE(TaskRemoveDevice)

 public:
  explicit TaskRemoveDevice(const QString& publickKey);
  ~TaskRemoveDevice();

  void run() override;

  DeletePolicy deletePolicy() const override { return NonDeletable; }

 private:
  QString m_publicKey;
  QByteArray m_authHeader;
};

#endif  // TASKREMOVEDEVICE_H
