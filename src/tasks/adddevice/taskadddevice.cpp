/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskadddevice.h"
#include "curve25519.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QRandomGenerator>

namespace {

Logger logger(LOG_MAIN, "TaskAddDevice");

QByteArray generatePrivateKey() {
  QByteArray key;

  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  for (uint8_t i = 0; i < CURVE25519_KEY_SIZE; ++i) {
    quint32 v = generator->generate();
    key.append(v & 0xFF);
  }

  return key.toBase64();
}

}  // anonymous namespace

TaskAddDevice::TaskAddDevice(const QString& deviceName)
    : Task("TaskAddDevice"), m_deviceName(deviceName) {
  MVPN_COUNT_CTOR(TaskAddDevice);
}

TaskAddDevice::~TaskAddDevice() { MVPN_COUNT_DTOR(TaskAddDevice); }

void TaskAddDevice::run(MozillaVPN* vpn) {
  logger.log() << "Adding the device" << m_deviceName;

  QByteArray privateKey = generatePrivateKey();
  QByteArray publicKey = Curve25519::generatePublicKey(privateKey);

#ifdef QT_DEBUG
  logger.log() << "Private key: " << privateKey;
  logger.log() << "Public key: " << publicKey;
#endif

  NetworkRequest* request =
      NetworkRequest::createForDeviceCreation(this, m_deviceName, publicKey);

  connect(request, &NetworkRequest::requestFailed,
          [this, vpn](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to add the device" << error;
            vpn->errorHandle(ErrorHandler::toErrorType(error));
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, vpn, publicKey, privateKey](const QByteArray&) {
            logger.log() << "Device added";
            vpn->deviceAdded(m_deviceName, publicKey, privateKey);
            emit completed();
          });
}
