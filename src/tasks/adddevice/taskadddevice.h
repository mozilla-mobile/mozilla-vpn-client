/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include <QObject>

#include "errorhandler.h"
#include "models/apierror.h"
#include "models/device.h"
#include "task.h"

class TaskAddDevice final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskAddDevice)

 public:
  TaskAddDevice(const QString& deviceName, const QString& deviceID,
                ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy =
                    ErrorHandler::PropagateError,
                const QString& token = QString());
  ~TaskAddDevice();

  void run() override;

  DeletePolicy deletePolicy() const override { return NonDeletable; }
  const ApiError& error() const { return m_apiError; }
  const Device& device() const { return m_device; }

 signals:
  void deviceAdded(const QString& deviceName, const QString& publicKey,
                   const QString& privateKey);

 private:
  QString m_deviceName;
  QString m_deviceID;
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::PropagateError;
  QString m_token;
  Device m_device;
  ApiError m_apiError;
};

#endif  // TASKADDDEVICE_H
