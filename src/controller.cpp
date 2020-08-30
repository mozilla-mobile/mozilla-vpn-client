#include "controller.h"
#include "mozillavpn.h"
#include "server.h"
#include "timercontroller.h"

#ifdef __linux__
#include "platforms/linux/linuxcontroller.h"
#else
#include "platforms/dummy/dummycontroller.h"
#endif

#include <QDebug>
#include <QTimer>

Controller::Controller()
{
    m_impl.reset(new TimerController(
#ifdef __linux__
        new LinuxController()
#else
        new DummyController()
#endif
            ));

    connect(m_impl.get(), &ControllerImpl::connected, this, &Controller::connected);
    connect(m_impl.get(), &ControllerImpl::disconnected, this, &Controller::disconnected);

    connect(&m_timer, &QTimer::timeout, this, &Controller::timeUpdated);
}

void Controller::setVPN(MozillaVPN *vpn)
{
    Q_ASSERT(!m_vpn);
    m_vpn = vpn;
}

void Controller::activate()
{
    qDebug() << "Activation" << m_state;

    if (m_state != StateOff && m_state != StateSwitching) {
        qDebug() << "Already disconnected";
        return;
    }

    QList<Server> servers = m_vpn->getServers();
    Q_ASSERT(!servers.isEmpty());

    m_currentServer = Server::weightChooser(servers);
    Q_ASSERT(m_currentServer.initialized());

    const Device *device = m_vpn->deviceModel()->currentDevice();

    if (m_state == StateOff) {
        m_state = StateConnecting;
        emit stateChanged();
    }

    m_timer.stop();

    m_impl->activate(m_currentServer, device, m_vpn->keys(), m_state == StateSwitching);
}

void Controller::deactivate()
{
    qDebug() << "Deactivation" << m_state;

    if (m_state != StateOn && m_state != StateSwitching) {
        qDebug() << "Already disconnected";
        return;
    }

    Q_ASSERT(m_state == StateOn || m_state == StateSwitching);

    if (m_state == StateOn) {
        m_state = StateDisconnecting;
        emit stateChanged();
    }

    m_timer.stop();

    m_connectionHealth.stop();

    const Device *device = m_vpn->deviceModel()->currentDevice();

    Q_ASSERT(m_currentServer.initialized());
    m_impl->deactivate(m_currentServer, device, m_vpn->keys(), m_state == StateSwitching);
}

void Controller::connected() {
    qDebug() << "Connected";

    Q_ASSERT(m_state == StateConnecting || m_state == StateSwitching);
    m_state = StateOn;
    emit stateChanged();

    m_time = 0;
    emit timeChanged();

    if (m_nextStep != None) {
        disconnect();
        return;
    }

    Q_ASSERT(!m_timer.isActive());
    m_timer.start(1000);

    m_connectionHealth.start(m_currentServer);
}

void Controller::disconnected() {
    qDebug() << "Disconnected";

    Q_ASSERT(m_state == StateDisconnecting || m_state == StateConnecting
             || m_state == StateSwitching);

    NextStep nextStep = m_nextStep;
    m_nextStep = None;

    if (nextStep == Quit) {
        emit readyToQuit();
        return;
    }

    if (nextStep == None && m_state == StateSwitching) {
        m_vpn->changeServer(m_switchingCountryCode, m_switchingCity);
        activate();
        return;
    }

    m_state = StateOff;
    emit stateChanged();
}

void Controller::timeUpdated() {
    Q_ASSERT(m_state == StateOn);

    ++m_time;
    emit timeChanged();
}

void Controller::changeServer(const QString &countryCode, const QString &city)
{
    Q_ASSERT(m_state == StateOn || m_state == StateOff);

    if (m_vpn->currentServer()->countryCode() == countryCode
        && m_vpn->currentServer()->city() == city) {
        qDebug() << "No server change needed";
        return;
    }

    if (m_state == StateOff) {
        qDebug() << "Change server";
        m_vpn->changeServer(countryCode, city);
        return;
    }

    m_timer.stop();

    qDebug() << "Switching to a different server";

    m_state = StateSwitching;

    m_currentCity = m_vpn->currentServer()->city();
    m_switchingCountryCode = countryCode;
    m_switchingCity = city;

    emit stateChanged();

    deactivate();
}

void Controller::quit()
{
    qDebug() << "Quitting";

    if (m_state == StateOff) {
        emit readyToQuit();
        return;
    }

    m_nextStep = Quit;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}

void Controller::logout()
{
    qDebug() << "Logout";

    m_vpn->logout();

    if (m_state == StateOff) {
        return;
    }

    m_nextStep = Disconnect;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}
