/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummypingsendworker.h"
#include "logger.h"

#include <QTimer>

constexpr uint32_t SHORT_PING_TIMER_MSEC = 200;
constexpr uint32_t LONG_PING_TIMER_MSEC = 6000;

namespace {
Logger logger(LOG_NETWORKING, "DummyPingSendWorker");
}

DummyPingSendWorker::DummyPingSendWorker(State state) : m_state(state) {}

void DummyPingSendWorker::sendPing(const QString &destination)
{
    logger.log() << "Dummy ping to:" << destination;

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setSingleShot(true);
        connect(m_timer, &QTimer::timeout, this, &DummyPingSendWorker::pingSucceeded);
    }

    switch (m_state) {
    case Stable:
        m_timer->start(SHORT_PING_TIMER_MSEC);
        break;
    case Unstable:
        m_timer->start(LONG_PING_TIMER_MSEC);
        break;
    case NoSignal:
        // Nothing to do.
        break;
    }
}

void DummyPingSendWorker::stopPing()
{
    if (m_timer) {
        m_timer->stop();
    }
}
