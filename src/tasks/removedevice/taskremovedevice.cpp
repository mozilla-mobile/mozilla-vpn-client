#include "taskremovedevice.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "userdata.h"

#include <QDebug>

TaskRemoveDevice::TaskRemoveDevice(const QString &deviceName)
    : Task("TaskRemoveDevice"), m_deviceName(deviceName)
{}

void TaskRemoveDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Removing the device" << m_deviceName;

    const DeviceData *device = vpn->userData()->device(m_deviceName);
    Q_ASSERT(device);

    QString publicKey = device->publicKey();

    NetworkRequest *request = NetworkRequest::createForDeviceRemoval(vpn, publicKey);

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to remove the device" << this << error;
        // TODO
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &) {
        qDebug() << "Device removed";
        vpn->deviceRemoved(m_deviceName);
        emit completed();
    });
}
