#include "macoscontroller.h"
#include "macosswiftcontroller.h"
#include "server.h"
#include "keys.h"
#include "device.h"

#include <QDebug>

void MacOSController::initialize(const Device *device, const Keys *keys) {
    MacOSSwiftController::initialize(
        device,
        keys,
        // initialize callback. This informs about the initialization steps.
                                     [this](bool status, Controller::State state, const QDateTime& date) {
            qDebug() << "Controller initialized. Connected status:" << status
                     << " and state:" << state << " and connection date:" << date;
            emit initialized(status, state, date);
        },
        // This monitors the connection VPN states.
        [this](Controller::State state) {
            qDebug() << "Something has changed from the outside:" << state;

            if (state == Controller::StateOff) {
                emit disconnected();
                return;
            }

            Q_ASSERT(state == Controller::StateOn);
            emit connected();
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

    MacOSSwiftController::activate(&server, [this]() {
        qDebug() << "Failure callback!";
        emit disconnected();
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

    MacOSSwiftController::deactivate();
}
