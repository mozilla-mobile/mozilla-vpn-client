#include "macoscontroller.h"
#include "macosswiftcontroller.h"
#include "server.h"
#include "keys.h"
#include "device.h"

#include <QDebug>

void MacOSController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_UNUSED(forSwitching);

    qDebug() << "MacOSController activating" << server.hostname();

    MacOSSwiftController::maybeInitializeController(device, keys, [this](bool status) {
        qDebug() << "Controller initialized" << status;

        if (!status) {
            emit disconnected();
            return;
        }

        MacOSSwiftController::controllerActivate([this](bool status) {
            qDebug() << "Activation result:" << status;

            if (!status) {
                emit disconnected();
                return;
            }

            emit connected();
        });
    });
}

void MacOSController::deactivate(const Server &server,
                                 const Device *device,
                                 const Keys *keys,
                                 bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "MacOSController deactivated" << server.hostname();

    emit disconnected();
}
