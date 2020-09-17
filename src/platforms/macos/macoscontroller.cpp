#include "macoscontroller.h"
#include "macosswiftcontroller.h"
#include "server.h"
#include "keys.h"
#include "device.h"

#include <QDebug>

void MacOSController::initialize(const Device *device, const Keys *keys) {
    MacOSSwiftController::maybeInitialize(device, keys, [this](bool status) {
        qDebug() << "Controller initialized" << status;
        emit initialized();
    });
}

void MacOSController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "MacOSController activating" << server.hostname();

    MacOSSwiftController::activate(&server, [this](bool status) {
        qDebug() << "Activation result:" << status;

        if (!status) {
            emit disconnected();
            return;
        }

        emit connected();
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

    MacOSSwiftController::deactivate([this](bool status) {
        qDebug() << "Deactivation result:" << status;
        emit disconnected();
    });
}
