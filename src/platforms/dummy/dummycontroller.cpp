#include "dummycontroller.h"
#include "server.h"

#include <QDebug>
#include <QRandomGenerator>

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

void DummyController::checkStatus()
{
    emit statusUpdated(QRandomGenerator::global()->generate() % 100000,
                       QRandomGenerator::global()->generate() % 100000);
}
