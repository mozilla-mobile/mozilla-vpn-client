/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "pingsender.h"

// In seconds, the timeout for unstable pings.
constexpr uint32_t TIMEOUT_UNSTABLE_SEC = 5;

// In seconds, the timeout to detect no-signal pings.
constexpr uint32_t TIMEOUT_NOSIGNAL_SEC = 30;

// Gap between 1 ping and the following one, in seconds.
constexpr uint32_t WAITING_TIMEOUT_SEC = 2;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionHealth");
}

ConnectionHealth::ConnectionHealth()
{
    m_pingSender = new PingSender(this);
    connect(m_pingSender, &PingSender::completed, this, &ConnectionHealth::pingCompleted);

    m_unstableTimer.setSingleShot(true);
    connect(&m_unstableTimer, &QTimer::timeout, [this]() {
        Q_ASSERT(m_state == Pending);
        logger.log() << "ConnectionHealth: timeout";
        m_state = Timeout;
        setStability(Unstable);
    });

    m_noSignalTimer.setSingleShot(true);
    connect(&m_noSignalTimer, &QTimer::timeout, [this]() {
        Q_ASSERT(m_state == Timeout);
        logger.log() << "ConnectionHealth: no signal";

        m_pingSender->stop();
        wait();
        setStability(NoSignal);
    });

    m_waitingTimer.setSingleShot(true);
    connect(&m_waitingTimer, &QTimer::timeout, [this]() {
        Q_ASSERT(m_state == Waiting);
        sendPing();
    });
}

void ConnectionHealth::start(const QString &serverIpv4Gateway)
{
    logger.log() << "ConnectionHealth activated for server:" << serverIpv4Gateway;

    setStability(Stable);

    m_gateway = serverIpv4Gateway;
    sendPing();
}

void ConnectionHealth::stop()
{
    logger.log() << "ConnectionHealth deactivated";
    m_state = Inactive;

    m_unstableTimer.stop();
    m_noSignalTimer.stop();
    m_waitingTimer.stop();
    m_pingSender->stop();
}

void ConnectionHealth::sendPing()
{
    Q_ASSERT(m_state == Waiting || m_state == Inactive);

    logger.log() << "ConnectionHealth: Sending a ping";

    m_state = Pending;
    m_pingSender->send(m_gateway);

    Q_ASSERT(!m_unstableTimer.isActive());
    m_unstableTimer.start(TIMEOUT_UNSTABLE_SEC * 1000);

    Q_ASSERT(!m_noSignalTimer.isActive());
    m_noSignalTimer.start(TIMEOUT_NOSIGNAL_SEC * 1000);

    Q_ASSERT(!m_waitingTimer.isActive());
}

void ConnectionHealth::pingCompleted()
{
    Q_ASSERT(m_state == Timeout || m_state == Pending);

    logger.log() << "ConnectionHealth: Ping completed";

    m_unstableTimer.stop();
    m_noSignalTimer.stop();

    State state = m_state;
    wait();

    if (state == Pending) {
        setStability(Stable);
    }
}

void ConnectionHealth::wait()
{
    Q_ASSERT(m_state == Timeout || m_state == Pending);

    logger.log() << "ConnectionHealth: Let's wait for the next ping to be sent";

    m_state = Waiting;
    Q_ASSERT(!m_waitingTimer.isActive());
    m_waitingTimer.start(WAITING_TIMEOUT_SEC * 1000);
}

void ConnectionHealth::setStability(ConnectionStability stability)
{
    logger.log() << "Stability changed:" << stability;

    m_stability = stability;
    emit stabilityChanged();
}

void ConnectionHealth::connectionStateChanged()
{
    logger.log() << "Connection state changed";

    if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
        stop();
        return;
    }

    MozillaVPN::instance()->controller()->getStatus(
        [this](const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes) {
            Q_UNUSED(txBytes);
            Q_UNUSED(rxBytes);

            if (!serverIpv4Gateway.isEmpty()) {
                start(serverIpv4Gateway);
            }
        });
}
