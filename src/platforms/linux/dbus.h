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

    void activate(const Server &server, const Device *device, const Keys *keys);
    void deactivate();
    void status();

signals:
    void connected();
    void disconnected();

    void failed();
    void statusReceived(const QString& status);

private:
    void monitorReply(QDBusPendingReply<bool> &reply);

private:
    OrgMozillaVpnDbusInterface *m_dbus;
};

#endif // DBUS_H
