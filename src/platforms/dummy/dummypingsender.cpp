#include "dummypingsender.h"

constexpr uint32_t SHORT_PING_TIMER_MSEC = 200;
constexpr uint32_t LONG_PING_TIMER_MSEC = 6000;

DummyPingSender::DummyPingSender(QObject *parent)
    : PingSender(parent),
      // Change this to "Unstable"or "NoSignal"to have different effects.
      m_state(Good)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &DummyPingSender::completed);
}

void DummyPingSender::send(const QString &destination)
{
    Q_UNUSED(destination)

    switch (m_state) {
    case Good:
        m_timer.start(SHORT_PING_TIMER_MSEC);
        break;
    case Unstable:
        m_timer.start(LONG_PING_TIMER_MSEC);
        break;
    case NoSignal:
        // Nothing to do.
        break;
    }
}

void DummyPingSender::stop()
{
    m_timer.stop();
}
