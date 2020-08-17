#include "taskadddevice.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "userdata.h"
#include "wireguardkeys.h"

#include <QDebug>

TaskAddDevice::TaskAddDevice() : Task("TaskAddDevice") {}

void TaskAddDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Maybe adding the device";

    QString deviceName = DeviceData::currentDeviceName();

    Q_ASSERT(vpn->userData());
    if (vpn->userData()->hasDevice(deviceName)) {
        vpn->deviceAdded(deviceName, QString(), QString());
        emit completed();
        return;
    }

    WireguardKeys *wg = WireguardKeys::generateKeys(this);

    connect(wg,
            &WireguardKeys::keysGenerated,
            [this, vpn, deviceName](const QString &privateKey, const QString &publicKey) {
                qDebug() << "Private key: " << privateKey;
                qDebug() << "Public key: " << publicKey;

                NetworkRequest *request = NetworkRequest::createForDeviceCreation(vpn,
                                                                                  deviceName,
                                                                                  publicKey);

                connect(request,
                        &NetworkRequest::requestFailed,
                        [this](QNetworkReply::NetworkError error) {
                            qDebug() << "Failed to add the device" << this << error;
                            // TODO
                        });

                connect(request,
                        &NetworkRequest::requestCompleted,
                        [this, vpn, deviceName, publicKey, privateKey](QByteArray) {
                            qDebug() << "Device added";
                            vpn->deviceAdded(deviceName, publicKey, privateKey);
                            emit completed();
                        });
            });
}
