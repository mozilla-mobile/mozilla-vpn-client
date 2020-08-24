#include "dummycontroller.h"

#include <QDebug>
#include <QTimer>

DummyController::DummyController()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
}

void DummyController::activate() {
    qDebug() << "DummyController activated";

    m_timer->stop();
    m_timer->start(2000);
    m_timer->disconnect();
    connect(m_timer, &QTimer::timeout, this, &ControllerImpl::connected);
}

void DummyController::deactivate() {
    qDebug() << "DummyController deactivated";

    m_timer->stop();
    m_timer->start(2000);
    m_timer->disconnect();
    connect(m_timer, &QTimer::timeout, this, &ControllerImpl::disconnected);
}
