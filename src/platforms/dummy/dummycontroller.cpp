/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

void DummyController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);

    qDebug() << "DummyController deactivated";

    emit disconnected();
}

void DummyController::checkStatus()
{
    m_txBytes += QRandomGenerator::global()->generate() % 100000;
    m_rxBytes += QRandomGenerator::global()->generate() % 100000;

    emit statusUpdated("127.0.0.1", m_txBytes, m_rxBytes);
}
