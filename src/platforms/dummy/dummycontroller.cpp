#include "dummycontroller.h"
#include "server.h"

#include <QDebug>

void DummyController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "DummyController activated" << server.hostname();

    emit connected();
}

void DummyController::deactivate(const Server &server,
                                 const Device *device,
                                 const Keys *keys,
                                 bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "DummyController deactivated" << server.hostname();

    emit disconnected();
}
