#include "taskadddevice.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>

void TaskAddDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Adding device: " << m_deviceName;

    NetworkRequest *request = NetworkRequest::createForDeviceCreation(vpn, m_deviceName, "TODO");

    connect(request, &NetworkRequest::requestFailed, [this](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to add the device" << this << error;
        // TODO
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](QByteArray) {
        qDebug() << "Device added";
        vpn->deviceAdded(m_deviceName);

        emit completed();
    });
}
