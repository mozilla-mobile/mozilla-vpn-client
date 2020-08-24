#include "controller.h"

#include <QDebug>

Controller::Controller()
    : m_state(StateOff) {}

void Controller::activate()
{
    Q_ASSERT(m_state == StateOff || m_state == StateConnecting);

    qDebug() << "Activation";

    m_state = StateConnecting;
    emit stateChanged();

}

void Controller::deactivate()
{
    qDebug() << "Deactivation";

    m_state = StateOff;
    emit stateChanged();
}
