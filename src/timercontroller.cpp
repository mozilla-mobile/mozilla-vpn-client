#include "timercontroller.h"

#include <QDebug>
#include <QTimer>

constexpr uint32_t TIME_ACTIVATION = 1000;
constexpr uint32_t TIME_DEACTIVATION = 1500;
constexpr uint32_t TIME_SWITCHING = 2000;

TimerController::TimerController(ControllerImpl *impl) : m_impl(impl)
{
    Q_ASSERT(m_impl);
    m_impl->setParent(this);

    connect(m_impl,
            &ControllerImpl::connected,
            this,
            &TimerController::maybeDone,
            Qt::QueuedConnection);
    connect(m_impl,
            &ControllerImpl::disconnected,
            this,
            &TimerController::maybeDone,
            Qt::QueuedConnection);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &TimerController::timeout);
}

void TimerController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               bool forSwitching)
{
    Q_ASSERT(m_state == None);
    m_state = Connecting;

    m_impl->activate(server, device, keys, forSwitching);

    if (!forSwitching) {
        m_timer->stop();
        m_timer->start(TIME_ACTIVATION);
    }
}

void TimerController::deactivate(const Server &server,
                                 const Device *device,
                                 const Keys *keys,
                                 bool forSwitching)
{
    Q_ASSERT(m_state == None);
    m_state = Disconnecting;

    m_impl->deactivate(server, device, keys, forSwitching);

    m_timer->stop();
    m_timer->start(forSwitching ? TIME_SWITCHING : TIME_DEACTIVATION);
}

void TimerController::timeout()
{
    qDebug() << "TimerController - Timeout:" << m_state;

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

void TimerController::maybeDone()
{
    qDebug() << "TimerController - Operation completed:" << m_state;

    Q_ASSERT(m_state == Connecting || m_state == Disconnecting);

    if (m_state == Connecting) {
        if (m_timer->isActive()) {
            // The connection was faster.
            m_state = Connected;
            return;
        }

        // The timer was faster.
        m_state = None;
        emit connected();
        return;
    }

    Q_ASSERT(m_state == Disconnecting);

    if (m_timer->isActive()) {
        // The disconnection was faster.
        m_state = Disconnected;
        return;
    }

    // The timer was faster.
    m_state = None;
    emit disconnected();
}
