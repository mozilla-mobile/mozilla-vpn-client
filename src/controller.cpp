#include "controller.h"
#include "mozillavpn.h"
#include "server.h"

#ifdef __linux__
#include "platforms/linux/linuxcontroller.h"
#else
#include "platforms/dummy/dummycontroller.h"
#endif

#include <QDebug>
#include <QTimer>

Controller::Controller()
{
#ifdef __linux__
    m_impl.reset(new LinuxController());
#else
    m_impl.reset(new DummyController());
#endif

    connect(m_impl.get(), &ControllerImpl::connected, this, &Controller::connected);
    connect(m_impl.get(), &ControllerImpl::disconnected, this, &Controller::disconnected);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Controller::timeUpdated);
}

void Controller::setVPN(MozillaVPN *vpn)
{
    Q_ASSERT(!m_vpn);
    m_vpn = vpn;
}

void Controller::activate()
{
    qDebug() << "Activation";

    if (m_state != StateOff) {
        qDebug() << "Already disconnected";
        return;
    }

    QList<Server> servers = m_vpn->getServers();
    Q_ASSERT(!servers.isEmpty());

    Server selectedServer = Server::weightChooser(servers);

    const Device *device = m_vpn->deviceModel()->currentDevice();

    Q_ASSERT(m_state == StateOff);
    m_state = StateConnecting;
    emit stateChanged();

    m_timer->stop();

    m_impl->activate(selectedServer, device, m_vpn->keys());
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

    QList<Server> servers = m_vpn->getServers();
    Q_ASSERT(!servers.isEmpty());

    Server selectedServer = Server::weightChooser(servers);

    const Device *device = m_vpn->deviceModel()->currentDevice();

    m_impl->deactivate(selectedServer, device, m_vpn->keys());
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

    Q_ASSERT(m_state == StateDisconnecting || m_state == StateConnecting);

    m_state = StateOff;
    emit stateChanged();
}

void Controller::timeUpdated() {
    Q_ASSERT(m_state == StateOn);

    ++m_time;
    emit timeChanged();
}
