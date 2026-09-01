/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskadddevice.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>

#include "constants.h"
#include "curve25519.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/apierror.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {

Logger logger("TaskAddDevice");

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

TaskAddDevice::TaskAddDevice(const QString& deviceName, const QString& deviceID)
    : Task("TaskAddDevice"), m_deviceName(deviceName), m_deviceID(deviceID) {
  MZ_COUNT_CTOR(TaskAddDevice);
}

TaskAddDevice::TaskAddDevice(const QString& deviceName, const QString& deviceID,
                             bool shared, const QString& token)
    : Task("TaskAddDevice"),
      m_deviceName(deviceName),
      m_deviceID(deviceID),
      m_shared(shared),
      m_token(token) {
  MZ_COUNT_CTOR(TaskAddDevice);
}

TaskAddDevice::~TaskAddDevice() { MZ_COUNT_DTOR(TaskAddDevice); }

void TaskAddDevice::run() {
  logger.debug() << "Adding the device" << logger.sensitive(m_deviceName);

  m_privateKey = generatePrivateKey();
  m_publicKey = Curve25519::generatePublicKey(m_privateKey);

  logger.debug() << "Private key: " << logger.sensitive(m_privateKey);
  logger.debug() << "Public key: " << logger.sensitive(m_publicKey);

  NetworkRequest* request = new NetworkRequest(this, 201);
  if (!m_token.isEmpty()) {
    request->auth(QByteArray("Bearer ") + m_token.toLocal8Bit());
  } else {
    request->auth();
  }
  request->post(Constants::apiUrl(Constants::Device),
                QJsonObject{{"name", m_deviceName},
                            {"unique_id", m_deviceID},
                            {"pubkey", m_publicKey}});
  request->disableTimeout();

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to add the device" << error;
            if (!m_shared) {
              REPORTNETWORKERROR(error, ErrorHandler::PropagateError, name());
            } else {
              ApiError apiError;
              // how do i handle an invalid response from the server?
              // m_apiError and m_error?
              Q_ASSERT(apiError.fromJson(data));
              m_apiError = apiError;
            }
            emit completed();
          });

  connect(
      request, &NetworkRequest::requestCompleted, this,
      [this](const QByteArray& data) {
        logger.debug() << "Device added";
        if (!m_shared) {
          MozillaVPN* vpn = MozillaVPN::instance();
          Q_ASSERT(vpn);
          vpn->deviceAdded(m_deviceName, m_publicKey, m_privateKey);
        } else {
          // how do i handle an invalid response from the server?
          // m_apiError and m_error?
          Q_ASSERT(m_device.fromJson(QJsonDocument::fromJson(data).object()));
        }
        emit completed();
      });
}
