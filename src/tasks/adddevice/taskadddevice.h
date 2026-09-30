/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include <QObject>

#include "models/apierror.h"
#include "models/devicemodel.h"
#include "task.h"

class TaskAddDevice final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAddDevice)

 public:
  TaskAddDevice(const QString& deviceName, const QString& deviceID);
  TaskAddDevice(const QString& deviceName, const QString& deviceID, bool shared,
                const QString& token);
  ~TaskAddDevice();

  void run() override;

  DeletePolicy deletePolicy() const override { return NonDeletable; }
  const QString& publicKey() const { return m_publicKey; }
  QString privateKey() const { return QString(m_privateKey); }
  const ApiError& error() const { return m_apiError; }
  const Device& device() const { return m_device; }

 private:
  QString m_deviceName;
  QString m_deviceID;
  Device m_device;
  QString m_publicKey;
  QByteArray m_privateKey;
  bool m_shared = false;
  QString m_token;
  ApiError m_apiError;
};

#endif  // TASKADDDEVICE_H
