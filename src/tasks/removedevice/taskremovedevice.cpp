/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskremovedevice.h"
#include "errorhandler.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskRemoveDevice");
}

TaskRemoveDevice::TaskRemoveDevice(const QString &deviceName)
    : Task("TaskRemoveDevice"), m_deviceName(deviceName)
{}

void TaskRemoveDevice::run(MozillaVPN *vpn)
{
    logger.log() << "Removing the device" << m_deviceName;

    const Device *device = vpn->deviceModel()->device(m_deviceName);
    Q_ASSERT(device);

    QString publicKey = device->publicKey();

    NetworkRequest *request = NetworkRequest::createForDeviceRemoval(this, vpn, publicKey);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        logger.log() << "Failed to remove the device" << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &) {
        logger.log() << "Device removed";
        vpn->deviceRemoved(m_deviceName);
        emit completed();
    });
}
