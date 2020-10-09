/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"
#include "controllergetstatushelper.h"
#include "controllerimpl.h"
#include "mozillavpn.h"
#include "server.h"
#include "timercontroller.h"

#ifdef __linux__
#include "platforms/linux/linuxcontroller.h"
#elif MACOS_INTEGRATION
#include "platforms/macos/macoscontroller.h"
#elif IOS_INTEGRATION
#include "platforms/macos/macoscontroller.h"
#else
#include "platforms/dummy/dummycontroller.h"
#endif

#include <QDebug>
#include <QTimer>

constexpr const uint32_t TIMER_MSEC = 1000;

Controller::Controller()
{
    m_impl.reset(new TimerController(
#ifdef __linux__
        new LinuxController()
#elif MACOS_INTEGRATION
        new MacOSController()
#elif IOS_INTEGRATION
        new MacOSController()
#else
        new DummyController()
#endif
            ));

    connect(m_impl.get(), &ControllerImpl::connected, this, &Controller::connected);
    connect(m_impl.get(), &ControllerImpl::disconnected, this, &Controller::disconnected);
    connect(m_impl.get(), &ControllerImpl::initialized, this, &Controller::implInitialized);

    connect(&m_timer, &QTimer::timeout, this, &Controller::timerTimeout);
}

Controller::~Controller() = default;

void Controller::initialize()
{
    qDebug() << "Initializing the controller";

    if (m_state == StateInitializing) {
        MozillaVPN *vpn = MozillaVPN::instance();
        Q_ASSERT(vpn);

        const Device *device = vpn->deviceModel()->currentDevice();
        m_impl->initialize(device, vpn->keys());
    }
}

void Controller::implInitialized(bool status, State state, const QDateTime &connectionDate)
{
    qDebug() << "Controller activated with status:" << status << "state:" << state
             << "connectionDate:" << connectionDate;

    Q_ASSERT(m_state == StateInitializing);

    if (!status) {
        MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
    }

    Q_UNUSED(status);

    if (processNextStep()) {
        return;
    }

    setState(state);

    // If we are connected already at startup time, we can trigger the connection sequence of tasks.
    if (state == StateOn) {
        connected();
        m_connectionDate = connectionDate;
        return;
    }

    if (MozillaVPN::instance()->settingsHolder()->startAtBoot()) {
        qDebug() << "Start on boot";
        activate();
    }
}

void Controller::activate()
{
    qDebug() << "Activation" << m_state;

    if (m_state != StateOff && m_state != StateSwitching) {
        qDebug() << "Already disconnected";
        return;
    }

    MozillaVPN *vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    QList<Server> servers = vpn->getServers();
    Q_ASSERT(!servers.isEmpty());

    Server server = Server::weightChooser(servers);
    Q_ASSERT(server.initialized());

    const Device *device = vpn->deviceModel()->currentDevice();

    if (m_state == StateOff) {
        setState(StateConnecting);
    }

    m_timer.stop();

    m_connectionDate = QDateTime::currentDateTime();

    m_impl->activate(server, device, vpn->keys(), m_state == StateSwitching);
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
        setState(StateDisconnecting);
    }

    m_timer.stop();
    m_impl->deactivate(m_state == StateSwitching);
}

void Controller::connected()
{
    qDebug() << "Connected from state:" << m_state;

    // This is an unexpected connection. Let's use the Connecting state to animate the UI.
    if (m_state != StateConnecting && m_state != StateSwitching) {
        setState(StateConnecting);

        m_connectionDate = QDateTime::currentDateTime();

        QTimer::singleShot(TIME_ACTIVATION, [this]() {
            if (m_state == StateConnecting) {
                connected();
            }
        });
        return;
    }

    setState(StateOn);
    emit timeChanged();

    if (m_nextStep != None) {
        disconnect();
        return;
    }

    m_timer.start(TIMER_MSEC);
}

void Controller::disconnected() {
    qDebug() << "Disconnected from state:" << m_state;

    m_timer.stop();

    // This is an unexpected disconnection. Let's use the Disconnecting state to animate the UI.
    if (m_state != StateDisconnecting && m_state != StateSwitching) {
        setState(StateDisconnecting);
        QTimer::singleShot(TIME_DEACTIVATION, [this]() {
            if (m_state == StateDisconnecting) {
                disconnected();
            }
        });
        return;
    }

    NextStep nextStep = m_nextStep;

    if (processNextStep()) {
        return;
    }

    if (nextStep == None && m_state == StateSwitching) {
        MozillaVPN::instance()->changeServer(m_switchingCountryCode, m_switchingCity);
        activate();
        return;
    }

    setState(StateOff);
}

void Controller::timerTimeout()
{
    Q_ASSERT(m_state == StateOn);
    emit timeChanged();
}

void Controller::changeServer(const QString &countryCode, const QString &city)
{
    Q_ASSERT(m_state == StateOn || m_state == StateOff);

    MozillaVPN *vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    if (vpn->currentServer()->countryCode() == countryCode && vpn->currentServer()->city() == city) {
        qDebug() << "No server change needed";
        return;
    }

    if (m_state == StateOff) {
        qDebug() << "Change server";
        vpn->changeServer(countryCode, city);
        return;
    }

    m_timer.stop();

    qDebug() << "Switching to a different server";

    m_currentCity = vpn->currentServer()->city();
    m_switchingCountryCode = countryCode;
    m_switchingCity = city;

    setState(StateSwitching);

    deactivate();
}

void Controller::quit()
{
    qDebug() << "Quitting";

    if (m_state == StateInitializing || m_state == StateOff || m_state == StateDeviceLimit) {
        emit readyToQuit();
        return;
    }

    m_nextStep = Quit;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}

void Controller::updateRequired()
{
    qDebug() << "Update required";

    if (m_state == StateOff) {
        emit readyToUpdate();
        return;
    }

    m_nextStep = Update;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}

void Controller::subscriptionNeeded()
{
    qDebug() << "Subscription needed";

    if (m_state == StateOff) {
        emit readyToSubscribe();
        return;
    }

    m_nextStep = Subscribe;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}

void Controller::logout()
{
    qDebug() << "Logout";

    MozillaVPN::instance()->logout();

    if (m_state == StateOff) {
        return;
    }

    m_nextStep = Disconnect;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}

void Controller::setDeviceLimit(bool deviceLimit)
{
    qDebug() << "Device limit mode:" << deviceLimit;

    if (!deviceLimit) {
        Q_ASSERT(m_state == StateDeviceLimit);
        setState(StateOff);
        return;
    }

    if (m_state == StateOff) {
        setState(StateDeviceLimit);
        return;
    }

    m_nextStep = DeviceLimit;

    if (m_state == StateOn) {
        deactivate();
        return;
    }
}

bool Controller::processNextStep()
{
    NextStep nextStep = m_nextStep;
    m_nextStep = None;

    if (nextStep == Quit) {
        emit readyToQuit();
        return true;
    }

    if (nextStep == Update) {
        emit readyToUpdate();
        return true;
    }

    if (nextStep == Subscribe) {
        emit readyToSubscribe();
        return true;
    }

    if (nextStep == DeviceLimit) {
        setState(StateDeviceLimit);
        return true;
    }

    return false;
}

void Controller::setState(State state)
{
    m_state = state;
    emit stateChanged();
}

int Controller::time() const
{
    return m_connectionDate.msecsTo(QDateTime::currentDateTime()) / 1000;
}

void Controller::captivePortalDetected()
{
    qDebug() << "Captive portal detected in state:" << m_state;
    // TODO: here we should disconnect the VPN and reconnect when the captive-portal-detection returns OK
}

void Controller::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);
    m_impl->getBackendLogs(std::move(callback));
}

void Controller::getStatus(
    std::function<void(const QString &serverIpv4Gateway, uint64_t txByte, uint64_t rxBytes)>
        &&a_callback)
{
    qDebug() << "check status";

    std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>
        callback = std::move(a_callback);

    if (m_state != StateOn) {
        callback(QString(), 0, 0);
        return;
    }

    ControllerGetStatusHelper *helper = new ControllerGetStatusHelper(this, std::move(callback));
    connect(m_impl.get(),
            &ControllerImpl::statusUpdated,
            helper,
            &ControllerGetStatusHelper::statusUpdated);

    m_impl->checkStatus();
}
