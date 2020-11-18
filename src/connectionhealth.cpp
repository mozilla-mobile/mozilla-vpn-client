/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionhealth.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "pingsender.h"

// Any X seconds, a new ping.
constexpr uint32_t PING_TIMOUT_SEC = 2;

// How many concurrent pings
constexpr int PINGS_MAX = 10;

// In seconds, the timeout for unstable pings.
constexpr uint32_t PING_TIME_UNSTABLE_NSEC = 5;

// In seconds, the timeout to detect no-signal pings.
constexpr uint32_t PING_TIME_NOSIGNAL_SEC = 30;

namespace {
Logger logger(LOG_NETWORKING, "ConnectionHealth");
}

ConnectionHealth::ConnectionHealth()
{
    connect(&m_pingTimer, &QTimer::timeout, this, &ConnectionHealth::nextPing);
    connect(&m_noSignalTimer, &QTimer::timeout, this, &ConnectionHealth::noSignalDetected);

    m_pingThread.start();
}

ConnectionHealth::~ConnectionHealth()
{
    m_pingThread.quit();
    m_pingThread.wait();
}

void ConnectionHealth::start(const QString &serverIpv4Gateway)
{
    logger.log() << "ConnectionHealth activated for server:" << serverIpv4Gateway;

    setStability(Stable);

    m_gateway = serverIpv4Gateway;
    m_pingTimer.start(PING_TIMOUT_SEC * 1000);
}

void ConnectionHealth::stop()
{
    logger.log() << "ConnectionHealth deactivated";

    m_pingTimer.stop();

    for (PingSender *pingSender : m_pings) {
        pingSender->deleteLater();
    }
    m_pings.clear();
}

void ConnectionHealth::setStability(ConnectionStability stability)
{
    if (m_stability == stability) {
        return;
    }

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
                stop();
                start(serverIpv4Gateway);
            }
        });
}

void ConnectionHealth::nextPing()
{
    logger.log() << "Sending a new ping. Total:" << m_pings.length();

    if (!m_noSignalTimer.isActive()) {
        m_noSignalTimer.start(PING_TIME_NOSIGNAL_SEC * 1000);
    }

    PingSender *pingSender = new PingSender(this, &m_pingThread);
    connect(pingSender, &PingSender::completed, this, &ConnectionHealth::pingReceived);
    m_pings.append(pingSender);
    pingSender->send(m_gateway);

    while (m_pings.length() > PINGS_MAX) {
        m_pings.at(0)->deleteLater();
        m_pings.removeAt(0);
    }
}

void ConnectionHealth::pingReceived(PingSender *pingSender, uint32_t msec)
{
    logger.log() << "Ping answer reeived in msec:" << msec;

    // If a ping has been received, we have signal.
    m_noSignalTimer.stop();

    QMutableListIterator<PingSender *> i(m_pings);
    while (i.hasNext()) {
        PingSender *thisPingSender = i.next();
        if (thisPingSender != pingSender) {
            continue;
        }

        i.remove();
        break;
    }

    pingSender->deleteLater();

    if (msec < PING_TIME_UNSTABLE_NSEC * 1000) {
        setStability(Stable);
    } else {
        setStability(Unstable);
    }
}

void ConnectionHealth::noSignalDetected()
{
    logger.log() << "No signal detected";
    setStability(NoSignal);
}
