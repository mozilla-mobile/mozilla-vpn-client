/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "device.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>

#ifdef QT_DEBUG
#include <QRandomGenerator>
#endif

#ifdef MVPN_IOS
#include "platforms/ios/iosutils.h"
#elif MVPN_MACOS
#include "platforms/macos/macosutils.h"
#endif

QString Device::currentDeviceName()
{
    QString deviceName =

#ifdef MVPN_IOS
	IOSUtils::computerName();
#elif MVPN_MACOS
        // MacOS has a funny way to rename the hostname based on the network status.
        MacOSUtils::computerName();
#else
        QSysInfo::machineHostName() + " " + QSysInfo::productType() + " "
        + QSysInfo::productVersion();
#endif

    /*  If we want to generate a new device name at each execution, comment out this block:
#ifdef QT_DEBUG
    static quint32 uniqueId = 0;
    if (uniqueId == 0) {
        uniqueId = QRandomGenerator::global()->generate();
    }

    deviceName = QString("%1 %2").arg(deviceName).arg(uniqueId);
#endif
    */

    return deviceName;
}

bool Device::fromJson(const QJsonValue &json)
{
    if (!json.isObject()) {
        return false;
    }

    QJsonObject obj = json.toObject();

    QJsonValue name = obj.take("name");
    if (!name.isString()) {
        return false;
    }

    QJsonValue pubKey = obj.take("pubkey");
    if (!pubKey.isString()) {
        return false;
    }

    QJsonValue createdAt = obj.take("created_at");
    if (!createdAt.isString()) {
        return false;
    }

    QDateTime date = QDateTime::fromString(createdAt.toString(), Qt::ISODate);
    if (!date.isValid()) {
        return false;
    }

    QJsonValue ipv4Address = obj.take("ipv4_address");
    if (!ipv4Address.isString()) {
        return false;
    }

    QJsonValue ipv6Address = obj.take("ipv6_address");
    if (!ipv6Address.isString()) {
        return false;
    }

    m_deviceName = name.toString();
    m_createdAt = date;
    m_publicKey = pubKey.toString();
    m_ipv4Address = ipv4Address.toString();
    m_ipv6Address = ipv6Address.toString();

    return true;
}
