/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn.h"
#include "device.h"
#include "servercountrymodel.h"
#include "tasks/accountandservers/taskaccountandservers.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/function/taskfunction.h"
#include "tasks/removedevice/taskremovedevice.h"
#include "user.h"

#ifdef IOS_INTEGRATION
#include "platforms/ios/iaphandler.h"
#endif

#include <QDebug>
#include <QDesktopServices>
#include <QLocale>
#include <QPointer>
#include <QTimer>

// TODO: constexpr const char *API_URL_PROD = "https://fpn.firefox.com";
constexpr const char *API_URL_PROD = "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";

#ifdef QT_DEBUG
constexpr const char *API_URL_DEBUG = "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";
#endif

// in seconds, how often we should fetch the server list and the account.
constexpr const uint32_t SCHEDULE_ACCOUNT_AND_SERVERS_TIMER_SEC = 3600;

// in seconds, hide alerts
constexpr const uint32_t HIDE_ALERT_SEC = 4;

// The singleton.
static MozillaVPN *s_instance = nullptr;

// static
void MozillaVPN::createInstance(QObject *parent, QQmlApplicationEngine* engine)
{
    qDebug() << "Creating MozillaVPN singleton";

    Q_ASSERT(!s_instance);
    s_instance = new MozillaVPN(parent, engine);
    s_instance->initialize();
}

// static
void MozillaVPN::deleteInstance()
{
    qDebug() << "Deleting MozillaVPN singleton";

    Q_ASSERT(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

// static
MozillaVPN *MozillaVPN::instance()
{
    Q_ASSERT(s_instance);
    return s_instance;
}

MozillaVPN::MozillaVPN(QObject *parent, QQmlApplicationEngine *engine)
    : QObject(parent), m_engine(engine)
{
    m_controller.setVPN(this);
    m_releaseMonitor.setVPN(this);

    connect(&m_alertTimer, &QTimer::timeout, [this]() { setAlert(NoAlert); });

    connect(&m_accountAndServersTimer, &QTimer::timeout, [this]() {
        scheduleTask(new TaskAccountAndServers());
    });

    connect(&m_controller, &Controller::readyToUpdate, [this]() { setState(StateUpdateRequired); });
    connect(&m_controller, &Controller::readyToSubscribe, [this]() {
        setState(StateSubscriptionNeeded);
    });
    connect(&m_controller, &Controller::initialized, [this]() { setState(StateMain); });

    connect(&m_localizer, &Localizer::languageChanged, [this](const QString &language) {
        qDebug() << "Storing the language:" << language;
        m_settingsHolder.setLanguage(language);
    });

    connect(&m_settingsHolder,
            &SettingsHolder::captivePortalAlertChanged,
            [this](bool captivePortalAlert) {
                if (!captivePortalAlert) {
                    m_captivePortalDetection.stop();
                }
            });
}

MozillaVPN::~MozillaVPN() = default;

void MozillaVPN::initialize()
{
    qDebug() << "MozillaVPN Initialization";

    // This is our first state.
    Q_ASSERT(m_state == StateInitialize);

    // API URL depends on the type of build.
    m_apiUrl = API_URL_PROD;
#ifdef QT_DEBUG
    m_apiUrl = API_URL_DEBUG;
#endif

    m_releaseMonitor.runSoon();

    m_localizer.initialize(m_settingsHolder.language());

    if (!m_settingsHolder.hasToken()) {
        return;
    }

    qDebug() << "We have a valid token";
    if (!m_user.fromSettings(m_settingsHolder)) {
        return;
    }

    if (!m_serverCountryModel.fromSettings(m_settingsHolder)) {
        qDebug() << "No server list found";
        m_settingsHolder.clear();
        return;
    }

    if (!m_deviceModel.fromSettings(m_settingsHolder)) {
        qDebug() << "No devices found";
        m_settingsHolder.clear();
        return;
    }

    if (!m_deviceModel.hasDevice(Device::currentDeviceName())) {
        qDebug() << "The current device has not been found";
        m_settingsHolder.clear();
        return;
    }

    if (!m_keys.fromSettings(m_settingsHolder)) {
        qDebug() << "No keys found";
        m_settingsHolder.clear();
        return;
    }

    Q_ASSERT(!m_serverData.initialized());
    if (!m_serverData.fromSettings(m_settingsHolder)) {
        m_serverCountryModel.pickRandom(m_serverData);
        Q_ASSERT(m_serverData.initialized());
        m_serverData.writeSettings(m_settingsHolder);
    }

    m_token = m_settingsHolder.token();

    scheduleTask(new TaskAccountAndServers());

    setState(StateMain);
    setUserAuthenticated(true);
}

void MozillaVPN::setState(State state)
{
    qDebug() << "Set state:" << state;
    m_state = state;
    emit stateChanged();

    // If we are activating the app, let's initialize the controller.
    if (m_state == StateMain) {
        m_connectionDataHolder.enable();
        m_controller.initialize();
        startSchedulingAccountAndServers();
    } else {
        m_connectionDataHolder.disable();
        stopSchedulingAccountAndServers();
    }
}

void MozillaVPN::authenticate()
{
    qDebug() << "Authenticate";

    setState(StateAuthenticating);

    hideAlert();

    scheduleTask(new TaskAuthenticate());
}

void MozillaVPN::openLink(LinkType linkType)
{
    qDebug() << "Opening link: " << linkType;

    QString url;

    switch (linkType) {
    case LinkAccount:
        url = getApiUrl();
        url.append("/r/vpn/account");
        break;

    case LinkContact:
        url = getApiUrl();
        url.append("/r/vpn/contact");
        break;

    case LinkFeedback:
        url = getApiUrl();
        url.append("/r/vpn/client/feedback");
        break;

    case LinkHelpSupport:
        url = getApiUrl();
        url.append("/r/vpn/support");
        break;

    case LinkTermsOfService:
        url = getApiUrl();
        url.append("/r/vpn/terms");
        break;

    case LinkPrivacyPolicy:
        url = getApiUrl();
        url.append("/r/vpn/privacy");
        break;

    case LinkUpdate:
        url = getApiUrl();
        // TODO
        break;

    default:
        qFatal("Unsupported link type!");
        return;
    }

    QDesktopServices::openUrl(url);
}

void MozillaVPN::scheduleTask(Task *task)
{
    Q_ASSERT(task);
    qDebug() << "Scheduling task: " << task->name();

    m_tasks.append(task);
    maybeRunTask();
}

void MozillaVPN::maybeRunTask()
{
    qDebug() << "Tasks: " << m_tasks.size();

    if (m_task_running || m_tasks.empty()) {
        return;
    }

    m_task_running = true;
    QPointer<Task> task = m_tasks.takeFirst();
    Q_ASSERT(!task.isNull());

    QObject::connect(task, &Task::completed, this, [this]() {
        qDebug() << "Task completed";

        m_task_running = false;
        maybeRunTask();
    });

    QObject::connect(task, &Task::completed, task, &Task::deleteLater);

    task->run(this);
}

void MozillaVPN::authenticationCompleted(const QByteArray &json, const QString &token)
{
    qDebug() << "Authentication completed";

    m_user.fromJson(json);
    m_user.writeSettings(m_settingsHolder);

    m_deviceModel.fromJson(json);
    m_deviceModel.writeSettings(m_settingsHolder);

    m_settingsHolder.setToken(token);
    m_token = token;

    setUserAuthenticated(true);

#ifdef IOS_INTEGRATION
    if (m_user.subscriptionNeeded()) {
        setState(StatePostAuthentication);
        return;
    }
#endif

    int deviceCount = m_deviceModel.activeDevices();

    QString deviceName = Device::currentDeviceName();
    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
        --deviceCount;
    }

    if (deviceCount >= m_user.maxDevices()) {
        // We need to go to "device limit" mode
        scheduleTask(new TaskFunction([this](MozillaVPN *) {
            if (m_state == StateAuthenticating) {
                m_controller.setDeviceLimit(true);
                setState(StatePostAuthentication);
            }
        }));

        // Nothing else to do.
        return;
    }

    // Here we add the current device.
    scheduleTask(new TaskAddDevice(deviceName));

    // Let's fetch the account and the servers.
    scheduleTask(new TaskAccountAndServers());

    // Finally we are able to activate the client.
    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        if (m_state == StateAuthenticating) {
            setState(StatePostAuthentication);
        }
    }));
}

void MozillaVPN::deviceAdded(const QString &deviceName,
                             const QString &publicKey,
                             const QString &privateKey)
{
    Q_UNUSED(publicKey);
    qDebug() << "Device added" << deviceName;

    m_settingsHolder.setPrivateKey(privateKey);
    m_keys.storeKey(privateKey);
}

void MozillaVPN::deviceRemoved(const QString &deviceName)
{
    qDebug() << "Device removed";

    m_deviceModel.removeDevice(deviceName);
}

void MozillaVPN::serversFetched(const QByteArray &serverData)
{
    qDebug() << "Server fetched!";

    m_serverCountryModel.fromJson(serverData);
    m_settingsHolder.setServers(serverData);

    // TODO: ... what about if the current server is gone?

    if (!m_serverData.initialized()) {
        m_serverCountryModel.pickRandom(m_serverData);
        Q_ASSERT(m_serverData.initialized());
        m_serverData.writeSettings(m_settingsHolder);
    }
}

void MozillaVPN::removeDevice(const QString &deviceName)
{
    qDebug() << "Remove device" << deviceName;

    // Let's inform the UI about what is going to happen.
    emit deviceRemoving(deviceName);

    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    if (!m_controller.isDeviceLimit()) {
        return;
    }

    // Let's recover from the device-limit mode.
    Q_ASSERT(!m_deviceModel.hasDevice(Device::currentDeviceName()));

    // Here we add the current device.
    scheduleTask(new TaskAddDevice(Device::currentDeviceName()));

    // Let's fetch the devices again.
    scheduleTask(new TaskAccountAndServers());

    // Finally we are able to activate the client.
    scheduleTask(new TaskFunction([this](MozillaVPN *) { m_controller.setDeviceLimit(false); }));
}

void MozillaVPN::accountChecked(const QByteArray &json)
{
    qDebug() << "Account checked";

    m_user.fromJson(json);
    m_user.writeSettings(m_settingsHolder);

    m_deviceModel.fromJson(json);
    m_deviceModel.writeSettings(m_settingsHolder);

    emit m_user.changed();

#ifdef IOS_INTEGRATION
    if (m_user.subscriptionNeeded() && m_state == StateMain) {
        m_controller.subscriptionNeeded();
    }
#endif
}

void MozillaVPN::cancelAuthentication()
{
    qDebug() << "Canceling authentication";

    if (m_state != StateAuthenticating) {
        // We cannot cancel tasks if we are not in authenticating state.
        return;
    }

    for (QList<QPointer<Task>>::Iterator i = m_tasks.begin(); i != m_tasks.end(); ++i) {
        delete *i;
    }

    m_tasks.clear();
    m_task_running = false;
    m_settingsHolder.clear();

    setState(StateInitialize);
}

void MozillaVPN::logout()
{
    qDebug() << "Logout";

    setAlert(LogoutAlert);

    // update-required state is the only one we want to keep when logging out.
    if (m_state != StateUpdateRequired) {
        setState(StateInitialize);
    }

    setUserAuthenticated(false);

    QString deviceName = Device::currentDeviceName();
    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        qDebug() << "Cleaning up all";
        m_settingsHolder.clear();
        m_keys.forgetKey();
        m_serverData.forget();
    }));
}

void MozillaVPN::setAlert(AlertType alert)
{
    m_alertTimer.stop();

    if (alert != NoAlert) {
        m_alertTimer.start(1000 * HIDE_ALERT_SEC);
    }

    m_alert = alert;
    emit alertChanged();
}

void MozillaVPN::errorHandle(ErrorHandler::ErrorType error)
{
    qDebug() << "Handling error" << error;

    Q_ASSERT(error != ErrorHandler::NoError);

    AlertType alert = NoAlert;

    switch (error) {
    case ErrorHandler::ConnectionFailureError:
        alert = ConnectionFailedAlert;
        break;

    case ErrorHandler::NoConnectionError:
        alert = NoConnectionAlert;
        break;

    case ErrorHandler::AuthenticationError:
        alert = AuthenticationFailedAlert;
        break;

    case ErrorHandler::BackendServiceError:
        alert = BackendServiceErrorAlert;
        break;

    default:
        break;
    }

    setAlert(alert);

    qDebug() << "Alert:" << alert << "State:" << m_state;

    if (alert == NoAlert) {
        return;
    }

    // Any error in authenticating state sends to the Initial state.
    if (m_state == StateAuthenticating) {
        setState(StateInitialize);
        return;
    }

    if (alert == AuthenticationFailedAlert) {
        m_settingsHolder.clear();
        setState(StateInitialize);
        return;
    }
}

const QList<Server> MozillaVPN::getServers() const
{
    return m_serverCountryModel.getServers(m_serverData);
}

void MozillaVPN::changeServer(const QString &countryCode, const QString &city)
{
    m_serverData.update(countryCode, city);
    m_serverData.writeSettings(m_settingsHolder);
}

void MozillaVPN::postAuthenticationCompleted()
{
    qDebug() << "Post authentication completed";

    // Super racy, but it could happen that we are already in update-required state.
    if (m_state == StateUpdateRequired) {
        return;
    }

#ifdef IOS_INTEGRATION
    if (m_user.subscriptionNeeded()) {
        setState(StateSubscriptionNeeded);
        return;
    }
#endif

    setState(StateMain);
}

void MozillaVPN::setUpdateRecommended(bool value)
{
    m_updateRecommended = value;
    emit updateRecommendedChanged();
}

void MozillaVPN::setUserAuthenticated(bool state)
{
    qDebug() << "User authentication state:" << state;
    m_userAuthenticated = state;
    emit userAuthenticationChanged();
}

void MozillaVPN::startSchedulingAccountAndServers()
{
    qDebug() << "Start scheduling account and servers";
    m_accountAndServersTimer.start(SCHEDULE_ACCOUNT_AND_SERVERS_TIMER_SEC * 1000);
}

void MozillaVPN::stopSchedulingAccountAndServers()
{
    qDebug() << "Stop scheduling account and servers";
    m_accountAndServersTimer.stop();
}

void MozillaVPN::subscribe()
{
    qDebug() << "Subscription required";

#ifdef IOS_INTEGRATION
    IAPHandler *iap = new IAPHandler(this);

    connect(iap, &IAPHandler::completed, [this]() {
        qDebug() << "Subscription completed";
        scheduleTask(new TaskAccountAndServers());
    });

    connect(iap, &IAPHandler::failed, [] { qDebug() << "Subscription failed"; });

    iap->start();
#endif
}
