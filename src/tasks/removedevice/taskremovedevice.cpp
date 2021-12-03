/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskremovedevice.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskRemoveDevice");
}

TaskRemoveDevice::TaskRemoveDevice(const QString& publicKey)
    : Task("TaskRemoveDevice"), m_publicKey(publicKey) {
  MVPN_COUNT_CTOR(TaskRemoveDevice);
}

TaskRemoveDevice::~TaskRemoveDevice() {
  MVPN_COUNT_DTOR(TaskRemoveDevice);

  // Nothing guarantees that when this task is deleted, the VPN object is still
  // alive. We cannot use the QObject-parenting solution because it deletes the
  // parent before the children.
  MozillaVPN* vpn = MozillaVPN::maybeInstance();
  if (vpn) {
    vpn->deviceRemovalCompleted(m_publicKey);
  }
}

void TaskRemoveDevice::run() {
  logger.debug() << "Removing the device with public key" << m_publicKey;

  NetworkRequest* request =
      NetworkRequest::createForDeviceRemoval(this, m_publicKey);

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Failed to remove the device" << error;
        MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
      });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray&) {
            logger.debug() << "Device removed";
            MozillaVPN::instance()->deviceRemoved(m_publicKey);
            emit completed();
          });
}
