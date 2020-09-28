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
                           server.publicKey(),
                           server.ipv4AddrIn(),
                           server.ipv6AddrIn(),
                           server.choosePort(),
                           MozillaVPN::instance()->settingsHolder()->ipv6());
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
