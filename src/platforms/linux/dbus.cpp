/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbus.h"
#include "device.h"
#include "ipaddressrange.h"
#include "keys.h"
#include "logger.h"
#include "mozillavpn.h"
#include "server.h"

#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

constexpr const char *DBUS_SERVICE = "org.mozilla.vpn.dbus";
constexpr const char *DBUS_PATH = "/";

namespace {
Logger logger(LOG_LINUX, "DBus");
}

DBus::DBus(QObject *parent) : QObject(parent)
{
    m_dbus = new OrgMozillaVpnDbusInterface(DBUS_SERVICE,
                                            DBUS_PATH,
                                            QDBusConnection::systemBus(),
                                            this);

    connect(m_dbus, &OrgMozillaVpnDbusInterface::connected, this, &DBus::connected);
    connect(m_dbus, &OrgMozillaVpnDbusInterface::disconnected, this, &DBus::disconnected);
}

QDBusPendingCallWatcher *DBus::version()
{
    logger.log() << "Version via DBus";
    QDBusPendingReply<QString> reply = m_dbus->version();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}

QDBusPendingCallWatcher *DBus::activate(const Server &server,
                                        const Device *device,
                                        const Keys *keys,
                                        const QList<IPAddressRange> &allowedIPAddressRanges)
{
    QJsonObject json;
    json.insert("privateKey", QJsonValue(keys->privateKey()));
    json.insert("deviceIpv4Address", QJsonValue(device->ipv4Address()));
    json.insert("deviceIpv6Address", QJsonValue(device->ipv6Address()));
    json.insert("serverIpv4Gateway", QJsonValue(server.ipv4Gateway()));
    json.insert("serverIpv6Gateway", QJsonValue(server.ipv6Gateway()));
    json.insert("serverPublicKey", QJsonValue(server.publicKey()));
    json.insert("serverIpv4AddrIn", QJsonValue(server.ipv4AddrIn()));
    json.insert("serverIpv6AddrIn", QJsonValue(server.ipv6AddrIn()));
    json.insert("serverPort", QJsonValue((double) server.choosePort()));
    json.insert("ipv6Enabled", QJsonValue(MozillaVPN::instance()->settingsHolder()->ipv6Enabled()));

    QJsonArray allowedIPAddesses;
    for (QList<IPAddressRange>::ConstIterator i = allowedIPAddressRanges.begin();
         i != allowedIPAddressRanges.end();
         ++i) {
        QJsonObject range;
        range.insert("address", QJsonValue(i->ipAddress()));
        range.insert("range", QJsonValue((double) i->range()));
        range.insert("isIpv6", QJsonValue(i->type() == IPAddressRange::IPv6));
        allowedIPAddesses.append(range);
    };
    json.insert("allowedIPAddressRanges", allowedIPAddesses);

    logger.log() << "Activate via DBus";
    QDBusPendingReply<bool> reply = m_dbus->activate(QJsonDocument(json).toJson(QJsonDocument::Compact));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}

QDBusPendingCallWatcher *DBus::deactivate()
{
    logger.log() << "Deactivate via DBus";
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
    logger.log() << "Status via DBus";
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
    logger.log() << "Logs via DBus";
    QDBusPendingReply<QString> reply = m_dbus->logs();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     watcher,
                     &QDBusPendingCallWatcher::deleteLater);
    return watcher;
}
