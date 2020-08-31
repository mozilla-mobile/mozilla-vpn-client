#include "dummypingsendworker.h"

#include <QDebug>
#include <QTimer>

constexpr uint32_t SHORT_PING_TIMER_MSEC = 200;
constexpr uint32_t LONG_PING_TIMER_MSEC = 6000;

DummyPingSendWorker::DummyPingSendWorker()
    : // Change this to "Unstable"or "NoSignal" to have different effects.
      m_state(Good)
{}

void DummyPingSendWorker::sendPing(const QString &destination)
{
    qDebug() << "Dummy ping to:" << destination;

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setSingleShot(true);
        connect(m_timer, &QTimer::timeout, this, &DummyPingSendWorker::pingSucceeded);
    }

    switch (m_state) {
    case Good:
        qDebug() << m_timer;
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
