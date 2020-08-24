#include "controller.h"
#include "platforms/dummy/dummycontroller.h"

#include <QDebug>
#include <QTimer>

Controller::Controller()
    : m_state(StateOff), m_timer(nullptr), m_time(0) {
    m_impl.reset(new DummyController());
    connect(m_impl.get(), &ControllerImpl::connected, this, &Controller::connected);
    connect(m_impl.get(), &ControllerImpl::disconnected, this, &Controller::disconnected);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Controller::timeUpdated);
}

void Controller::activate()
{
    qDebug() << "Activation";

    if (m_state != StateOff) {
        qDebug() << "Already disconnected";
        return;
    }

    Q_ASSERT(m_state == StateOff);
    m_state = StateConnecting;
    emit stateChanged();

    m_timer->stop();

    m_impl->activate();
}

void Controller::deactivate()
{
    qDebug() << "Deactivation";

    if (m_state != StateOn) {
        qDebug() << "Already disconnected";
        return;
    }

    Q_ASSERT(m_state == StateOn);
    m_state = StateDisconnecting;
    emit stateChanged();

    m_timer->stop();

    m_impl->deactivate();
}

void Controller::connected() {
    qDebug() << "Connected";

    Q_ASSERT(m_state == StateConnecting);
    m_state = StateOn;
    emit stateChanged();

    m_time = 0;
    emit timeChanged();

    Q_ASSERT(!m_timer->isActive());
    m_timer->start(1000);
}

void Controller::disconnected() {
    qDebug() << "Disconnected";

    Q_ASSERT(m_state == StateDisconnecting);
    m_state = StateOff;
    emit stateChanged();
}

void Controller::timeUpdated() {
    Q_ASSERT(m_state == StateOn);

    ++m_time;
    emit timeChanged();
}
