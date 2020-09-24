#include "linuxcontroller.h"
#include "device.h"
#include "errorhandler.h"
#include "keys.h"
#include "mozillavpn.h"
#include "server.h"
#include "wgquickprocess.h"

#ifdef USE_POLKIT
#include "dbus.h"
#endif

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

#ifdef USE_POLKIT
    DBus *dbus = new DBus(this);

    connect(dbus, &DBus::failed, [this]() {
        MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
        emit disconnected();
    });

    connect(dbus, &DBus::succeeded, this, &LinuxController::connected);

    dbus->activate(server, device, keys);
#else
    WgQuickProcess *wgQuick = new WgQuickProcess(WgQuickProcess::Up);

    connect(wgQuick, &WgQuickProcess::failed, this, &LinuxController::disconnected);
    connect(wgQuick, &WgQuickProcess::succeeded, this, &LinuxController::connected);

    wgQuick->run(keys->privateKey(),
                 device->ipv4Address(),
                 device->ipv6Address(),
                 server.ipv4Gateway(),
                 server.publicKey(),
                 server.ipv4AddrIn(),
                 server.ipv6AddrIn(),
                 server.choosePort(),
                 MozillaVPN::instance()->settingsHolder()->ipv6());
#endif
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

#ifdef USE_POLKIT
    DBus *dbus = new DBus(this);

    connect(dbus, &DBus::failed, this, &LinuxController::disconnected);
    connect(dbus, &DBus::succeeded, this, &LinuxController::disconnected);

    dbus->deactivate(server, device, keys);
#else
    WgQuickProcess *wgQuick = new WgQuickProcess(WgQuickProcess::Down);

    connect(wgQuick, &WgQuickProcess::failed, this, &LinuxController::disconnected);
    connect(wgQuick, &WgQuickProcess::succeeded, this, &LinuxController::disconnected);

    wgQuick->run(keys->privateKey(),
                 device->ipv4Address(),
                 device->ipv6Address(),
                 server.ipv4Gateway(),
                 server.publicKey(),
                 server.ipv4AddrIn(),
                 server.ipv6AddrIn(),
                 server.choosePort(),
                 MozillaVPN::instance()->settingsHolder()->ipv6());
#endif
}
