#include "taskadddevice.h"
#include "curve25519/curve25519.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>

TaskAddDevice::TaskAddDevice(const QString &deviceName)
    : Task("TaskAddDevice"), m_deviceName(deviceName)
{}

void TaskAddDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Adding the device" << m_deviceName;

    QByteArray privateKey = Curve25519::generatePrivateKey();
    QByteArray publicKey = Curve25519::generatePublicKey(privateKey);

    qDebug() << "Private key: " << privateKey;
    qDebug() << "Public key: " << publicKey;

    NetworkRequest *request = NetworkRequest::createForDeviceCreation(vpn, m_deviceName, publicKey);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to add the device" << this << error;
        // TODO
    });

    connect(request,
            &NetworkRequest::requestCompleted,
            [this, vpn, publicKey, privateKey](const QByteArray &) {
                qDebug() << "Device added";
                vpn->deviceAdded(m_deviceName, publicKey, privateKey);
                emit completed();
            });
}
