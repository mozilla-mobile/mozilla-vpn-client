/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUS_H
#define DBUS_H

#include "wgquickprocess.h"

#include <QObject>

class DbusAdaptor;

class DBus final : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mozilla.vpn.dbus")

public:
    DBus(QObject *parent);

    void setAdaptor(DbusAdaptor* adaptor);

    bool checkInterface();

public slots:
    QString version();

    bool activate(const QString &jsonConfig);

    bool deactivate();

    QString status();

    QString logs();

private:
    bool runWgQuick(WgQuickProcess::Op op,
                    const QString &privateKey,
                    const QString &deviceIpv4Address,
                    const QString &deviceIpv6Address,
                    const QString &serverIpv4Gateway,
                    const QString &serverIpv6Gateway,
                    const QString &serverPublicKey,
                    const QString &serverIpv4AddrIn,
                    const QString &serverIpv6AddrIn,
                    const QString &allowedIPAddressRange,
                    int serverPort,
                    bool ipv6Enabled);

private:
    DbusAdaptor *m_adaptor = nullptr;

    bool m_connected = false;
    QString m_lastPrivateKey;
    QString m_lastDeviceIpv4Address;
    QString m_lastDeviceIpv6Address;
    QString m_lastServerIpv4Gateway;
    QString m_lastServerIpv6Gateway;
    QString m_lastServerPublicKey;
    QString m_lastServerIpv4AddrIn;
    QString m_lastServerIpv6AddrIn;
    int m_lastServerPort = 0;
    bool m_lastIpv6Enabled = false;
    QString m_lastAllowedIPAddressRanges;
};

#endif // DBUS_H
