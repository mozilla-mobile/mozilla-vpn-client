/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummycontroller.h"
#include "logger.h"
#include "models/server.h"

#include <QRandomGenerator>

namespace {
Logger logger(LOG_CONTROLLER, "DummyController");
}

void DummyController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               const QList<IPAddressRange> &allowedIPAddressRanges,
                               bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(allowedIPAddressRanges);
    Q_UNUSED(forSwitching);

    logger.log() << "DummyController activated" << server.hostname();

    emit connected();
}

void DummyController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);

    logger.log() << "DummyController deactivated";

    emit disconnected();
}

void DummyController::checkStatus()
{
    m_txBytes += QRandomGenerator::global()->generate() % 100000;
    m_rxBytes += QRandomGenerator::global()->generate() % 100000;

    emit statusUpdated("127.0.0.1", m_txBytes, m_rxBytes);
}

void DummyController::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);
    callback("DummyController is always happy");
}

void DummyController::cleanupBackendLogs() {}
