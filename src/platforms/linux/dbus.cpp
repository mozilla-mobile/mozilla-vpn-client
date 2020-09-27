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
}

void DBus::activate(const Server &server, const Device *device, const Keys *keys)
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
    monitorReply(reply);
}

void DBus::deactivate()
{
    qDebug() << "Deactivate via DBus";
    QDBusPendingReply<bool> reply = m_dbus->deactivate();
    monitorReply(reply);
}

void DBus::status()
{
    qDebug() << "Status via DBus";
    QDBusPendingReply<QString> reply = m_dbus->status();

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     [this](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<QString> reply = *call;
                         if (reply.isError()) {
                             qDebug() << "Error received from the DBus service";
                             emit failed();
                         } else {
                             emit statusReceived(reply.argumentAt<0>());
                         }

                         call->deleteLater();
                     });
}

void DBus::monitorReply(QDBusPendingReply<bool> &reply)
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     &QDBusPendingCallWatcher::finished,
                     [this](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<bool> reply = *call;
                         if (reply.isError()) {
                             qDebug() << "Error received from the DBus service";
                             emit failed();
                         } else {
                             bool status = reply.argumentAt<0>();
                             if (status) {
                                 qDebug() << "DBus service says: all good.";
                                 emit succeeded();
                             } else {
                                 qDebug() << "DBus service says: error.";
                                 emit failed();
                             }
                         }

                         call->deleteLater();
                     });
}
