/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVICE_H
#define DEVICE_H

#include <QDateTime>
#include <QList>

class QJsonValue;

class Device final
{
public:
    Device(const QString &deviceName,
           const QDateTime &createdAt,
           const QString &publicKey,
           const QString& ipv4Address,
           const QString& ipv6Address)
        : m_deviceName(deviceName), m_createdAt(createdAt), m_publicKey(publicKey), m_ipv4Address(ipv4Address), m_ipv6Address(ipv6Address)
    {}

    static QString currentDeviceName();

    static Device fromJson(const QJsonValue &json);

    const QString &name() const { return m_deviceName; }

    const QDateTime &createdAt() const { return m_createdAt; }

    bool isDevice(const QString &deviceName) const { return m_deviceName == deviceName; }

    const QString &publicKey() const { return m_publicKey; }

    const QString& ipv4Address() const { return m_ipv4Address; }

    const QString& ipv6Address() const { return m_ipv6Address; }

private:
    QString m_deviceName;
    QDateTime m_createdAt;
    QString m_publicKey;
    QString m_ipv4Address;
    QString m_ipv6Address;
};

#endif // DEVICE_H
