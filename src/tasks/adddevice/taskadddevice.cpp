/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskadddevice.h"
#include "curve25519.h"
#include "errorhandler.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>
#include <QRandomGenerator>

namespace {

QByteArray generatePrivateKey()
{
    QByteArray key;

    QRandomGenerator *generator = QRandomGenerator::system();
    Q_ASSERT(generator);

    for (uint8_t i = 0; i < CURVE25519_KEY_SIZE; ++i) {
        quint32 v = generator->generate();
        key.append(v & 0xFF);
    }

    return key.toBase64();
}

} // anonymous namespace

TaskAddDevice::TaskAddDevice(const QString &deviceName)
    : Task("TaskAddDevice"), m_deviceName(deviceName)
{}

void TaskAddDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Adding the device" << m_deviceName;

    QByteArray privateKey = generatePrivateKey();
    QByteArray publicKey = Curve25519::generatePublicKey(privateKey);

    qDebug() << "Private key: " << privateKey;
    qDebug() << "Public key: " << publicKey;

    NetworkRequest *request = NetworkRequest::createForDeviceCreation(vpn, m_deviceName, publicKey);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to add the device" << this << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
    });

    connect(request,
            &NetworkRequest::requestCompleted,
            [this, vpn, publicKey, privateKey](const QByteArray &) {
                qDebug() << "Device added";
                vpn->deviceAdded(m_deviceName, publicKey, privateKey);
                emit completed();
            });
}
