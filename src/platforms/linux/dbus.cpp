/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbus.h"
#include "device.h"
#include "keys.h"
#include "mozillavpn.h"
#include "server.h"

#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

constexpr const char *DBUS_SERVICE = "org.mozilla.vpn.dbus";
constexpr const char *DBUS_PATH = "/";

DBus::DBus(QObject *parent) : QObject(parent)
{
    m_dbus = new OrgMozillaVpnDbusInterface(DBUS_SERVICE,
                                            DBUS_PATH,
                                            QDBusConnection::systemBus(),
                                            this);

    connect(m_dbus, &OrgMozillaVpnDbusInterface::connected, this, &DBus::connected);
    connect(m_dbus, &OrgMozillaVpnDbusInterface::disconnected, this, &DBus::disconnected);
}

QDBusPendingCallWatcher *DBus::activate(const Server &server, const Device *device, const Keys *keys)
{
    qDebug() << "Activate via DBus";
    QDBusPendingReply<bool> reply
        = m_dbus->activate(keys->privateKey(),
                           device->ipv4Address(),
                           device->ipv6Address(),
                           server.ipv4Gateway(),
                           server.ipv6Gateway(),
                           server.publicKey(),
                           server.ipv4AddrIn(),
                           server.ipv6AddrIn(),
                           server.choosePort(),
                           MozillaVPN::instance()->settingsHolder()->ipv6Enabled(),
                           MozillaVPN::instance()->settingsHolder()->localNetworkAccess());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}

QDBusPendingCallWatcher *DBus::deactivate()
{
    qDebug() << "Deactivate via DBus";
    QDBusPendingReply<bool> reply = m_dbus->deactivate();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}

QDBusPendingCallWatcher *DBus::status()
{
    qDebug() << "Status via DBus";
    QDBusPendingReply<QString> reply = m_dbus->status();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}

QDBusPendingCallWatcher *DBus::logs()
{
    qDebug() << "Logs via DBus";
    QDBusPendingReply<QString> reply = m_dbus->logs();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}
