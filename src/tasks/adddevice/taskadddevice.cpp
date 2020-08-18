#include "taskadddevice.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "wireguardkeys.h"

#include <QDebug>

TaskAddDevice::TaskAddDevice(const QString &deviceName)
    : Task("TaskAddDevice"), m_deviceName(deviceName)
{}

void TaskAddDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Adding the device" << m_deviceName;

    WireguardKeys *wg = WireguardKeys::generateKeys(this);

    connect(wg,
            &WireguardKeys::keysGenerated,
            [this, vpn](const QString &privateKey, const QString &publicKey) {
                qDebug() << "Private key: " << privateKey;
                qDebug() << "Public key: " << publicKey;

                NetworkRequest *request = NetworkRequest::createForDeviceCreation(vpn,
                                                                                  m_deviceName,
                                                                                  publicKey);

                connect(request,
                        &NetworkRequest::requestFailed,
                        [this](QNetworkReply::NetworkError error) {
                            qDebug() << "Failed to add the device" << this << error;
                            // TODO
                        });

                connect(request,
                        &NetworkRequest::requestCompleted,
                        [this, vpn, publicKey, privateKey](QByteArray) {
                            qDebug() << "Device added";
                            vpn->deviceAdded(m_deviceName, publicKey, privateKey);
                            emit completed();
                        });
            });
}
