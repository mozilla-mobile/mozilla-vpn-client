/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn.h"
#include "logger.h"
#include "loghandler.h"
#include "models/device.h"
#include "models/servercountrymodel.h"
#include "models/user.h"
#include "tasks/accountandservers/taskaccountandservers.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/function/taskfunction.h"
#include "tasks/removedevice/taskremovedevice.h"

#ifdef IOS_INTEGRATION
#include "platforms/ios/iaphandler.h"
#include "platforms/ios/iosdatamigration.h"
#include "platforms/ios/taskiosproducts.h"
#endif

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QPointer>
#include <QScreen>
#include <QTimer>
#include <QUrl>

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

namespace {
Logger logger(LOG_MAIN, "MozillaVPN");
}

// static
void MozillaVPN::createInstance(QObject *parent, QQmlApplicationEngine *engine, bool startMinimized)
{
    logger.log() << "Creating MozillaVPN singleton";

    Q_ASSERT(!s_instance);
    s_instance = new MozillaVPN(parent, engine, startMinimized);
    s_instance->initialize();
}

// static
void MozillaVPN::deleteInstance()
{
    logger.log() << "Deleting MozillaVPN singleton";

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

MozillaVPN::MozillaVPN(QObject *parent, QQmlApplicationEngine *engine, bool startMinimized)
    : QObject(parent), m_engine(engine), m_private(new Private()), m_startMinimized(startMinimized)
{
    connect(&m_alertTimer, &QTimer::timeout, [this]() { setAlert(NoAlert); });

    connect(&m_accountAndServersTimer, &QTimer::timeout, [this]() {
        scheduleTask(new TaskAccountAndServers());

#ifdef IOS_INTEGRATION
        scheduleTask(new TaskIOSProducts());
#endif
    });

    connect(&m_private->m_controller, &Controller::readyToUpdate, [this]() {
        setState(StateUpdateRequired);
    });
    connect(&m_private->m_controller, &Controller::readyToSubscribe, [this]() {
        setState(StateSubscriptionNeeded);
    });
    connect(&m_private->m_controller, &Controller::initialized, [this]() { setState(StateMain); });

    connect(&m_private->m_controller,
            &Controller::stateChanged,
            &m_private->m_connectionDataHolder,
            &ConnectionDataHolder::connectionStateChanged);

    connect(&m_private->m_controller,
            &Controller::stateChanged,
            &m_private->m_connectionHealth,
            &ConnectionHealth::connectionStateChanged);

    connect(&m_private->m_controller,
            &Controller::stateChanged,
            &m_private->m_captivePortalDetection,
            &CaptivePortalDetection::controllerStateChanged);

    connect(&m_private->m_settingsHolder,
            &SettingsHolder::captivePortalAlertChanged,
            &m_private->m_captivePortalDetection,
            &CaptivePortalDetection::settingsChanged);

    connect(&m_private->m_captivePortalDetection,
            &CaptivePortalDetection::captivePortalDetected,
            &m_private->m_controller,
            &Controller::captivePortalDetected);
}

MozillaVPN::~MozillaVPN()
{
    delete m_private;
}

void MozillaVPN::initialize()
{
    logger.log() << "MozillaVPN Initialization";

    // This is our first state.
    Q_ASSERT(m_state == StateInitialize);

    // API URL depends on the type of build.
    m_apiUrl = API_URL_PROD;
#ifdef QT_DEBUG
    m_apiUrl = API_URL_DEBUG;
#endif

    m_private->m_releaseMonitor.runSoon();

#ifdef IOS_INTEGRATION
    if (!m_private->m_settingsHolder.hasNativeIOSDataMigrated()) {
        IOSDataMigration::migrate();
        m_private->m_settingsHolder.setNativeIOSDataMigrated(true);
    }
#endif

    m_private->m_localizer.initialize(m_private->m_settingsHolder.languageCode());

    m_private->m_captivePortalDetection.initialize();

    if (!m_private->m_settingsHolder.hasToken()) {
        return;
    }

    logger.log() << "We have a valid token";
    if (!m_private->m_user.fromSettings(m_private->m_settingsHolder)) {
        return;
    }

    if (!m_private->m_serverCountryModel.fromSettings(m_private->m_settingsHolder)) {
        logger.log() << "No server list found";
        m_private->m_settingsHolder.clear();
        return;
    }

    if (!m_private->m_deviceModel.fromSettings(m_private->m_settingsHolder)) {
        logger.log() << "No devices found";
        m_private->m_settingsHolder.clear();
        return;
    }

    if (!m_private->m_deviceModel.hasDevice(Device::currentDeviceName())) {
        logger.log() << "The current device has not been found";
        m_private->m_settingsHolder.clear();
        return;
    }

    if (!m_private->m_keys.fromSettings(m_private->m_settingsHolder)) {
        logger.log() << "No keys found";
        m_private->m_settingsHolder.clear();
        return;
    }

    if (!modelsInitialized()) {
        logger.log() << "Models not initialized yet";
        m_private->m_settingsHolder.clear();
        return;
    }

    Q_ASSERT(!m_private->m_serverData.initialized());
    if (!m_private->m_serverData.fromSettings(m_private->m_settingsHolder)) {
        m_private->m_serverCountryModel.pickRandom(m_private->m_serverData);
        Q_ASSERT(m_private->m_serverData.initialized());
        m_private->m_serverData.writeSettings(m_private->m_settingsHolder);
    }

    m_token = m_private->m_settingsHolder.token();

    scheduleTask(new TaskAccountAndServers());

#ifdef IOS_INTEGRATION
    scheduleTask(new TaskIOSProducts());
#endif

    setState(StateMain);
    setUserAuthenticated(true);
}

void MozillaVPN::setState(State state)
{
    logger.log() << "Set state:" << state;
    m_state = state;
    emit stateChanged();

    // If we are activating the app, let's initialize the controller.
    if (m_state == StateMain) {
        m_private->m_connectionDataHolder.enable();
        m_private->m_controller.initialize();
        startSchedulingAccountAndServers();
    } else {
        m_private->m_connectionDataHolder.disable();
        stopSchedulingAccountAndServers();
    }
}

void MozillaVPN::authenticate()
{
    logger.log() << "Authenticate";

    setState(StateAuthenticating);

    hideAlert();

    scheduleTask(new TaskAuthenticate());
}

void MozillaVPN::openLink(LinkType linkType)
{
    logger.log() << "Opening link: " << linkType;

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
    logger.log() << "Scheduling task: " << task->name();

    m_tasks.append(task);
    maybeRunTask();
}

void MozillaVPN::maybeRunTask()
{
    logger.log() << "Tasks: " << m_tasks.size();

    if (m_task_running || m_tasks.empty()) {
        return;
    }

    m_task_running = true;
    QPointer<Task> task = m_tasks.takeFirst();
    Q_ASSERT(!task.isNull());

    QObject::connect(task, &Task::completed, this, &MozillaVPN::taskCompleted);
    QObject::connect(task, &Task::completed, task, &Task::deleteLater);

    task->run(this);
}

void MozillaVPN::taskCompleted()
{
    logger.log() << "Task completed";

    m_task_running = false;
    maybeRunTask();
}

void MozillaVPN::setToken(const QString &token)
{
    m_private->m_settingsHolder.setToken(token);
    m_token = token;
}

void MozillaVPN::authenticationCompleted(const QByteArray &json, const QString &token)
{
    logger.log() << "Authentication completed";

    if (!m_private->m_user.fromJson(json)) {
        logger.log() << "Failed to parse the User JSON data";
        errorHandle(ErrorHandler::BackendServiceError);
        return;
    }

    if (!m_private->m_deviceModel.fromJson(json)) {
        logger.log() << "Failed to parse the DeviceModel JSON data";
        errorHandle(ErrorHandler::BackendServiceError);
        return;
    }

    m_private->m_user.writeSettings(m_private->m_settingsHolder);
    m_private->m_deviceModel.writeSettings(m_private->m_settingsHolder);

    setToken(token);
    setUserAuthenticated(true);

#ifdef IOS_INTEGRATION
    if (m_private->m_user.subscriptionNeeded()) {
        scheduleTask(new TaskIOSProducts());
        scheduleTask(new TaskFunction([this](MozillaVPN*) {
            setState(StatePostAuthentication);
        }));
        return;
    }
#endif

    int deviceCount = m_private->m_deviceModel.activeDevices();

    QString deviceName = Device::currentDeviceName();
    if (m_private->m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
        --deviceCount;
    }

    if (deviceCount >= m_private->m_user.maxDevices()) {
        // We need to go to "device limit" mode
        scheduleTask(new TaskFunction([this](MozillaVPN *) {
            if (m_state == StateAuthenticating) {
                m_private->m_controller.setDeviceLimit(true);
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

#ifdef IOS_INTEGRATION
    scheduleTask(new TaskIOSProducts());
#endif

    // Finally we are able to activate the client.
    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        if (m_state != StateAuthenticating) {
            return;
        }

        if (!modelsInitialized()) {
            logger.log() << "Failed to complete the authentication";
            errorHandle(ErrorHandler::BackendServiceError);
            return;
        }

        Q_ASSERT(m_private->m_serverData.initialized());

        setState(StatePostAuthentication);
    }));
}

void MozillaVPN::deviceAdded(const QString &deviceName,
                             const QString &publicKey,
                             const QString &privateKey)
{
    Q_UNUSED(publicKey);
    logger.log() << "Device added" << deviceName;

    m_private->m_settingsHolder.setPrivateKey(privateKey);
    m_private->m_keys.storeKey(privateKey);
}

void MozillaVPN::deviceRemoved(const QString &deviceName)
{
    logger.log() << "Device removed";

    m_private->m_deviceModel.removeDevice(deviceName);
}

bool MozillaVPN::setServerList(const QByteArray &serverData)
{
    if (!m_private->m_serverCountryModel.fromJson(serverData)) {
        logger.log() << "Failed to store the server-countries";
        return false;
    }

    m_private->m_settingsHolder.setServers(serverData);
    return true;
}

void MozillaVPN::serversFetched(const QByteArray &serverData)
{
    logger.log() << "Server fetched!";

    if (!setServerList(serverData)) {
        // This is OK. The check is done elsewhere.
        return;
    }

    // The serverData could be unset or invalid with the new server list.
    if (!m_private->m_serverData.initialized()
        || !m_private->m_serverCountryModel.exists(m_private->m_serverData)) {
        m_private->m_serverCountryModel.pickRandom(m_private->m_serverData);
        Q_ASSERT(m_private->m_serverData.initialized());
        m_private->m_serverData.writeSettings(m_private->m_settingsHolder);
    }
}

void MozillaVPN::removeDevice(const QString &deviceName)
{
    logger.log() << "Remove device" << deviceName;

    // Let's inform the UI about what is going to happen.
    emit deviceRemoving(deviceName);

    if (m_private->m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    if (m_private->m_controller.state() != Controller::StateDeviceLimit) {
        return;
    }

    // Let's recover from the device-limit mode.
    Q_ASSERT(!m_private->m_deviceModel.hasDevice(Device::currentDeviceName()));

    // Here we add the current device.
    scheduleTask(new TaskAddDevice(Device::currentDeviceName()));

    // Let's fetch the devices again.
    scheduleTask(new TaskAccountAndServers());

    // Finally we are able to activate the client.
    scheduleTask(
        new TaskFunction([this](MozillaVPN *) { m_private->m_controller.setDeviceLimit(false); }));
}

void MozillaVPN::accountChecked(const QByteArray &json)
{
    logger.log() << "Account checked";

    if (!m_private->m_user.fromJson(json)) {
        logger.log() << "Failed to parse the User JSON data";
        // We don't need to communicate it to the user. Let's ignore it.
        return;
    }

    if (!m_private->m_deviceModel.fromJson(json)) {
        logger.log() << "Failed to parse the DeviceModel JSON data";
        // We don't need to communicate it to the user. Let's ignore it.
        return;
    }

    m_private->m_user.writeSettings(m_private->m_settingsHolder);
    m_private->m_deviceModel.writeSettings(m_private->m_settingsHolder);

#ifdef IOS_INTEGRATION
    if (m_private->m_user.subscriptionNeeded() && m_state == StateMain) {
        m_private->m_controller.subscriptionNeeded();
    }
#endif

    // To test the subscription needed view, comment out this line:
    //m_controller.subscriptionNeeded();
}

void MozillaVPN::cancelAuthentication()
{
    logger.log() << "Canceling authentication";

    if (m_state != StateAuthenticating) {
        // We cannot cancel tasks if we are not in authenticating state.
        return;
    }

    for (QPointer<Task> &task : m_tasks) {
        delete task;
    }

    m_tasks.clear();
    m_task_running = false;
    m_private->m_settingsHolder.clear();

    setState(StateInitialize);
}

void MozillaVPN::logout()
{
    logger.log() << "Logout";

    setAlert(LogoutAlert);

    // update-required state is the only one we want to keep when logging out.
    if (m_state != StateUpdateRequired) {
        setState(StateInitialize);
    }

    setUserAuthenticated(false);

    QString deviceName = Device::currentDeviceName();
    if (m_private->m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        logger.log() << "Cleaning up all";
        m_private->m_settingsHolder.clear();
        m_private->m_keys.forgetKey();
        m_private->m_serverData.forget();
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
    logger.log() << "Handling error" << error;

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
        if (m_userAuthenticated) {
            alert = AuthenticationFailedAlert;
        }
        break;

    case ErrorHandler::BackendServiceError:
        alert = BackendServiceErrorAlert;
        break;

    default:
        break;
    }

    setAlert(alert);

    logger.log() << "Alert:" << alert << "State:" << m_state;

    if (alert == NoAlert) {
        return;
    }

    // Any error in authenticating state sends to the Initial state.
    if (m_state == StateAuthenticating) {
        setState(StateInitialize);
        return;
    }

    if (alert == AuthenticationFailedAlert) {
        m_private->m_controller.deactivate();
        m_private->m_settingsHolder.clear();
        setState(StateInitialize);
        return;
    }
}

const QList<Server> MozillaVPN::getServers() const
{
    return m_private->m_serverCountryModel.getServers(m_private->m_serverData);
}

void MozillaVPN::changeServer(const QString &countryCode, const QString &city)
{
    m_private->m_serverData.update(countryCode, city);
    m_private->m_serverData.writeSettings(m_private->m_settingsHolder);
}

void MozillaVPN::postAuthenticationCompleted()
{
    logger.log() << "Post authentication completed";

    // Super racy, but it could happen that we are already in update-required state.
    if (m_state == StateUpdateRequired) {
        return;
    }

#ifdef IOS_INTEGRATION
    if (m_private->m_user.subscriptionNeeded()) {
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
    logger.log() << "User authentication state:" << state;
    m_userAuthenticated = state;
    emit userAuthenticationChanged();
}

void MozillaVPN::startSchedulingAccountAndServers()
{
    logger.log() << "Start scheduling account and servers";
    m_accountAndServersTimer.start(SCHEDULE_ACCOUNT_AND_SERVERS_TIMER_SEC * 1000);
}

void MozillaVPN::stopSchedulingAccountAndServers()
{
    logger.log() << "Stop scheduling account and servers";
    m_accountAndServersTimer.stop();
}

void MozillaVPN::subscribe()
{
    logger.log() << "Subscription required";

#ifdef IOS_INTEGRATION
    IAPHandler *iap = new IAPHandler(this);

    connect(iap, &IAPHandler::completed, [this]() {
        logger.log() << "Subscription completed";
        scheduleTask(new TaskAccountAndServers());
    });

    connect(iap, &IAPHandler::failed, [] { logger.log() << "Subscription failed"; });

    iap->start();
#endif
}

bool MozillaVPN::writeAndShowLogs(QStandardPaths::StandardLocation location)
{
    return writeLogs(location, [](const QString &filename) {
        logger.log() << "Opening the logFile somehow:" << filename;
        QUrl url = QUrl::fromLocalFile(filename);
        QDesktopServices::openUrl(url);
    });
}

bool MozillaVPN::writeLogs(QStandardPaths::StandardLocation location,
                           std::function<void(const QString &filename)> &&a_callback)
{
    logger.log() << "Trying to save logs in:" << location;

    std::function<void(const QString &filename)> callback = std::move(a_callback);

    if (!QFileInfo::exists(QStandardPaths::writableLocation(location))) {
        return false;
    }

    QString filename;
    QDate now = QDate::currentDate();

    QTextStream(&filename) << "mozillavpn-" << now.year() << "-" << now.month() << "-" << now.day()
                           << ".txt";

    QDir logDir(QStandardPaths::writableLocation(location));
    QString logFile = logDir.filePath(filename);

    if (QFileInfo::exists(logFile)) {
        logger.log() << logFile << "exists. Let's try a new filename";

        for (uint32_t i = 1;; ++i) {
            QString filename;
            QTextStream(&filename) << "mozillavpn-" << now.year() << "-" << now.month() << "-"
                                   << now.day() << "_" << i << ".txt";
            logFile = logDir.filePath(filename);
            if (!QFileInfo::exists(logFile)) {
                logger.log() << "Filename found!" << i;
                break;
            }
        }
    }

    logger.log() << "Writing logs into: " << logFile;

    QFile file(logFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logger.log() << "Failed to open the logfile";
        return false;
    }

    QTextStream out(&file);

    out << "Mozilla VPN logs" << Qt::endl << "================" << Qt::endl << Qt::endl;

    LogHandler *logHandler = LogHandler::instance();
    for (const LogHandler::Log &log : logHandler->logs()) {
        logHandler->prettyOutput(out, log);
    }

    file.close();

    MozillaVPN::instance()->controller()->getBackendLogs(
        [callback = std::move(callback), logFile](const QString &logs) {
            logger.log() << "Logs from the backend service received";

            QFile file(logFile);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                logger.log() << "Failed to re-open the logfile";
                return;
            }

            QTextStream out(&file);

            out << Qt::endl
                << Qt::endl
                << "Mozilla VPN backend logs" << Qt::endl
                << "========================" << Qt::endl
                << Qt::endl;

            if (!logs.isEmpty()) {
                out << logs;
            } else {
                out << "No logs from the backend.";
            }

            file.close();

            callback(logFile);
        });

    return true;
}

void MozillaVPN::viewLogs()
{
    logger.log() << "View logs";

    if (writeAndShowLogs(QStandardPaths::DesktopLocation)) {
        return;
    }

    if (writeAndShowLogs(QStandardPaths::HomeLocation)) {
        return;
    }

    if (writeAndShowLogs(QStandardPaths::TempLocation)) {
        return;
    }

    qWarning() << "No Desktop, no Home, no Temp folder. Unable to store the log files.";
}

bool MozillaVPN::modelsInitialized() const
{
    return m_private->m_user.initialized() && m_private->m_serverCountryModel.initialized()
           && m_private->m_deviceModel.initialized()
           && m_private->m_deviceModel.hasDevice(Device::currentDeviceName())
           && m_private->m_keys.initialized();
}
