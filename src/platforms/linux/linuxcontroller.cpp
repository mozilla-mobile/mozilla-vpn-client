#include "linuxcontroller.h"
#include "wgquickprocess.h"

#include <QDebug>
#include <QProcess>

void LinuxController::activate(const Server &server, const Device* device, const Keys* keys)
{
    qDebug() << "LinuxController activated";

    WgQuickProcess *wgQuick = new WgQuickProcess(WgQuickProcess::Up);

    connect(wgQuick, &WgQuickProcess::failed, this, &LinuxController::disconnected);
    connect(wgQuick, &WgQuickProcess::succeeded, this, &LinuxController::connected);

    wgQuick->Run(server, device, keys);
}

void LinuxController::deactivate(const Server &server, const Device *device, const Keys *keys)
{
    qDebug() << "LinuxController deactivated";

    WgQuickProcess *wgQuick = new WgQuickProcess(WgQuickProcess::Down);

    connect(wgQuick, &WgQuickProcess::failed, this, &LinuxController::disconnected);
    connect(wgQuick, &WgQuickProcess::succeeded, this, &LinuxController::disconnected);

    wgQuick->Run(server, device, keys);
}
