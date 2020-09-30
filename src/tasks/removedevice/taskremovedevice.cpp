/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskremovedevice.h"
#include "errorhandler.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "user.h"

#include <QDebug>

TaskRemoveDevice::TaskRemoveDevice(const QString &deviceName)
    : Task("TaskRemoveDevice"), m_deviceName(deviceName)
{}

void TaskRemoveDevice::run(MozillaVPN *vpn)
{
    qDebug() << "Removing the device" << m_deviceName;

    const Device *device = vpn->deviceModel()->device(m_deviceName);
    Q_ASSERT(device);

    QString publicKey = device->publicKey();

    NetworkRequest *request = NetworkRequest::createForDeviceRemoval(vpn, publicKey);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "Failed to remove the device" << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &) {
        qDebug() << "Device removed";
        vpn->deviceRemoved(m_deviceName);
        emit completed();
    });
}
