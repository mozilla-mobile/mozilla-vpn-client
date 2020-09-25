#include "linuxcontroller.h"
#include "device.h"
#include "dbus.h"
#include "errorhandler.h"
#include "keys.h"
#include "mozillavpn.h"
#include "server.h"

#include <QDebug>
#include <QProcess>

void LinuxController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_ASSERT(device);
    Q_ASSERT(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "LinuxController activated";

    DBus *dbus = new DBus(this);

    connect(dbus, &DBus::failed, [this]() {
        MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
        emit disconnected();
    });

    connect(dbus, &DBus::succeeded, this, &LinuxController::connected);

    dbus->activate(server, device, keys);
}

void LinuxController::deactivate(const Server &server,
                                 const Device *device,
                                 const Keys *keys,
                                 bool forSwitching)
{
    Q_ASSERT(device);
    Q_ASSERT(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "LinuxController deactivated";

    DBus *dbus = new DBus(this);

    connect(dbus, &DBus::failed, this, &LinuxController::disconnected);
    connect(dbus, &DBus::succeeded, this, &LinuxController::disconnected);

    dbus->deactivate(server, device, keys);
}

void LinuxController::checkStatus()
{
    qDebug() << "Check status";
}
