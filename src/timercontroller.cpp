/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "timercontroller.h"
#include "logger.h"

namespace {
Logger logger(LOG_CONTROLLER, "TimerController");
}

TimerController::TimerController(ControllerImpl *impl) : m_impl(impl)
{
    Q_ASSERT(m_impl);
    m_impl->setParent(this);

    connect(m_impl, &ControllerImpl::initialized, this, &ControllerImpl::initialized);
    connect(m_impl,
            &ControllerImpl::connected,
            [this] { TimerController::maybeDone(true); });
    connect(m_impl,
            &ControllerImpl::disconnected,
            [this] { TimerController::maybeDone(false); });
    connect(m_impl,
            &ControllerImpl::statusUpdated, this, &ControllerImpl::statusUpdated);

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &TimerController::timeout);
}

void TimerController::initialize(const Device *device, const Keys *keys)
{
    m_impl->initialize(device, keys);
}

void TimerController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               const CaptivePortal &captivePortal,
                               bool forSwitching)
{
    Q_ASSERT(m_state == None);
    m_state = Connecting;

    if (!forSwitching) {
        m_timer.stop();
        m_timer.start(TIME_ACTIVATION);
    }

    m_impl->activate(server, device, keys, captivePortal, forSwitching);
}

void TimerController::deactivate(bool forSwitching)
{
    Q_ASSERT(m_state == None);
    m_state = Disconnecting;

    m_timer.stop();
    m_timer.start(forSwitching ? TIME_SWITCHING : TIME_DEACTIVATION);

    m_impl->deactivate(forSwitching);
}

void TimerController::timeout()
{
    logger.log() << "TimerController - Timeout:" << m_state;

    Q_ASSERT(m_state != None);

    if (m_state == Connected) {
        m_state = None;
        emit connected();
        return;
    }

    if (m_state == Disconnected) {
        m_state = None;
        emit disconnected();
        return;
    }

    // Any other state can be ignored.
}

void TimerController::maybeDone(bool isConnected)
{
    logger.log() << "TimerController - Operation completed:" << m_state << isConnected;

    if (m_state == Connecting) {
        if (m_timer.isActive()) {
            // The connection was faster.
            m_state = isConnected ? Connected : Disconnected;
            return;
        }

        // The timer was faster.
        m_state = None;

        if (isConnected) {
            emit connected();
        } else {
            emit disconnected();
        }
        return;
    }

    if (m_state == Disconnecting) {
        if (m_timer.isActive()) {
            // The disconnection was faster.
            m_state = Disconnected;
            return;
        }

        // The timer was faster.
        m_state = None;
        emit disconnected();
        return;
    }

    // External events could trigger the following codes.

    Q_ASSERT(m_state == None);
    Q_ASSERT(!m_timer.isActive());

    if (isConnected) {
        emit connected();
        return;
    }

    emit disconnected();
}

void TimerController::checkStatus()
{
    m_impl->checkStatus();
}

void TimerController::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);
    m_impl->getBackendLogs(std::move(callback));
}
