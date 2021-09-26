/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskremovedevice.h"
#include "core.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskRemoveDevice");
}

TaskRemoveDevice::TaskRemoveDevice(const QString& publicKey)
    : Task("TaskRemoveDevice"), m_publicKey(publicKey) {
  MVPN_COUNT_CTOR(TaskRemoveDevice);
}

TaskRemoveDevice::~TaskRemoveDevice() { MVPN_COUNT_DTOR(TaskRemoveDevice); }

void TaskRemoveDevice::run() {
  logger.debug() << "Removing the device with public key" << m_publicKey;

  NetworkRequest* request =
      NetworkRequest::createForDeviceRemoval(this, m_publicKey);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to remove the device" << error;
            Core::instance()->errorHandle(ErrorHandler::toErrorType(error));
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray&) {
            logger.debug() << "Device removed";
            Core::instance()->deviceRemoved(m_publicKey);
            emit completed();
          });
}
