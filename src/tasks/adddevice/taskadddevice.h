/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include "task.h"

#include <QObject>

class TaskAddDevice final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAddDevice)

 public:
  TaskAddDevice(const QString& deviceName, const QString& deviceID);
  ~TaskAddDevice();

  void run(MozillaVPN* vpn) override;

 private:
  QString m_deviceName;
  QString m_deviceID;
};

#endif  // TASKADDDEVICE_H
