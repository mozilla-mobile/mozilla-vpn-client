/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUS_H
#define DBUS_H

#include "dbus_interface.h"

#include <QObject>

class Server;
class Device;
class Keys;
class QDBusPendingCallWatcher;

class DBus : public QObject
{
    Q_OBJECT

public:
    DBus(QObject *parent);

    QDBusPendingCallWatcher *activate(const Server &server, const Device *device, const Keys *keys);
    QDBusPendingCallWatcher *deactivate();
    QDBusPendingCallWatcher *status();

signals:
    void connected();
    void disconnected();

private:
    OrgMozillaVpnDbusInterface *m_dbus;
};

#endif // DBUS_H
