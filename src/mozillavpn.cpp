#include "mozillavpn.h"
#include "device.h"
#include "servercountrymodel.h"
#include "tasks/account/taskaccount.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/fetchservers/taskfetchservers.h"
#include "tasks/function/taskfunction.h"
#include "tasks/removedevice/taskremovedevice.h"
#include "user.h"

#include <QDebug>
#include <QDesktopServices>
#include <QPointer>
#include <QTimer>

// TODO: constexpr const char *API_URL_PROD = "https://fpn.firefox.com";
constexpr const char *API_URL_PROD = "https://stage.guardian.nonprod.cloudops.mozgcp.net";

#ifdef QT_DEBUG
constexpr const char *API_URL_DEBUG = "https://stage.guardian.nonprod.cloudops.mozgcp.net";
#endif

constexpr const char *SETTINGS_TOKEN = "token";

// in seconds, how often we should fetch the server list.
constexpr const uint32_t SCHEDULE_SERVER_FETCH_TIMER_SEC = 3600;

// in seconds, how often we should check the account
constexpr const uint32_t SCHEDULE_ACCOUNT_CHECK_TIMER_SEC = 3600;

// in seconds, hide alerts
constexpr const uint32_t HIDE_ALERT_SEC = 4;

MozillaVPN::MozillaVPN(QObject *parent) : QObject(parent), m_settings("mozilla", "guardianvpn")
{
    m_controller.setVPN(this);
    m_releaseMonitor.setVPN(this);

    connect(&m_alertTimer, &QTimer::timeout, [this]() { setAlert(NoAlert); });
}

MozillaVPN::~MozillaVPN() = default;

void MozillaVPN::initialize(int &, char *[])
{
    qDebug() << "MozillaVPN Initialization";

    // This is our first state.
    m_state = StateInitialize;

    // API URL depends on the type of build.
    m_apiUrl = API_URL_PROD;
#ifdef QT_DEBUG
    m_apiUrl = API_URL_DEBUG;
#endif

    m_releaseMonitor.runSoon();

    if (!m_settings.contains(SETTINGS_TOKEN)) {
        return;
    }

    qDebug() << "We have a valid token";
    if (!m_user.fromSettings(m_settings)) {
        return;
    }

    if (!m_serverCountryModel.fromSettings(m_settings)) {
        qDebug() << "No server list found";
        m_settings.clear();
        return;
    }

    if (!m_deviceModel.fromSettings(m_settings)) {
        qDebug() << "No devices found";
        m_settings.clear();
        return;
    }

    if (!m_deviceModel.hasDevice(Device::currentDeviceName())) {
        qDebug() << "The current device has not been found";
        m_settings.clear();
        return;
    }

    if (!m_keys.fromSettings(m_settings)) {
        qDebug() << "No keys found";
        m_settings.clear();
        return;
    }

    Q_ASSERT(!m_serverData.initialized());
    if (!m_serverData.fromSettings(m_settings)) {
        m_serverCountryModel.pickRandom(m_serverData);
        Q_ASSERT(m_serverData.initialized());
        m_serverData.writeSettings(m_settings);
    }

    m_token = m_settings.value(SETTINGS_TOKEN).toString();

    scheduleTask(new TaskAccount());
    scheduleTask(new TaskFetchServers());

    setState(StateMain);
    setUserAuthenticated(true);
}

void MozillaVPN::setState(State state)
{
    qDebug() << "Set state:" << state;
    m_state = state;
    emit stateChanged();
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
        // TODO
        break;

    default:
        qFatal("Unsupported link type!");
        return;
    }

    QDesktopServices::openUrl(url);
}

void MozillaVPN::scheduleTask(Task* task)
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
    m_user.writeSettings(m_settings);

    m_deviceModel.fromJson(json);
    m_deviceModel.writeSettings(m_settings);

    m_settings.setValue(SETTINGS_TOKEN, token);
    m_token = token;

    setUserAuthenticated(true);

    // Then we fetch the list of servers.
    scheduleTask(new TaskFetchServers());

    int deviceCount = m_deviceModel.activeDevices();

    QString deviceName = Device::currentDeviceName();
    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
        --deviceCount;
    }

    if (deviceCount >= m_user.maxDevices()) {
        // We need to go to "device limit"mode
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

    // Let's fetch the devices again.
    scheduleTask(new TaskAccount());

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

    m_keys.storeKey(m_settings, privateKey);
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
    m_serverCountryModel.writeSettings(m_settings);

    // TODO: ... what about if the current server is gone?

    if (!m_serverData.initialized()) {
        m_serverCountryModel.pickRandom(m_serverData);
        Q_ASSERT(m_serverData.initialized());
        m_serverData.writeSettings(m_settings);
    }

    qDebug() << "Scheduling the server fetch";

    QTimer::singleShot(1000 * SCHEDULE_SERVER_FETCH_TIMER_SEC,
                       [this]() { scheduleTask(new TaskFetchServers()); });
}

void MozillaVPN::removeDevice(const QString &deviceName)
{
    qDebug() << "Remove device" << deviceName;

    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    if (!m_controller.isDeviceLimit()) {
        return;
    }

    // Let's recover from the device-limit mode.
    Q_ASSERT(!m_deviceModel.hasDevice(Device::currentDeviceName()));

    // Here we add the current device.
    scheduleTask(new TaskAddDevice(deviceName));

    // Let's fetch the devices again.
    scheduleTask(new TaskAccount());

    // Finally we are able to activate the client.
    scheduleTask(new TaskFunction([this](MozillaVPN *) { m_controller.setDeviceLimit(false); }));
}

void MozillaVPN::accountChecked(const QByteArray& json)
{
    qDebug() << "Account checked";

    m_user.fromJson(json);
    m_user.writeSettings(m_settings);

    m_deviceModel.fromJson(json);
    m_deviceModel.writeSettings(m_settings);

    emit m_user.changed();

    qDebug() << "Scheduling the account check";
    QTimer::singleShot(1000 * SCHEDULE_ACCOUNT_CHECK_TIMER_SEC,
                       [this]() { scheduleTask(new TaskAccount()); });
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

    m_task_running = false;
    m_tasks.clear();

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
    scheduleTask(new TaskRemoveDevice(deviceName));

    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        qDebug() << "Cleaning up all";
        m_settings.clear();
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

void MozillaVPN::errorHandle(QNetworkReply::NetworkError error) {
    qDebug() << "Handling error" << error;

    Q_ASSERT(error != QNetworkReply::NoError);

    AlertType alert = NoAlert;

    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        [[fallthrough]];
    case QNetworkReply::RemoteHostClosedError:
        [[fallthrough]];
    case QNetworkReply::SslHandshakeFailedError:
        [[fallthrough]];
    case QNetworkReply::TemporaryNetworkFailureError:
        [[fallthrough]];
    case QNetworkReply::NetworkSessionFailedError:
        [[fallthrough]];
    case QNetworkReply::TooManyRedirectsError:
        [[fallthrough]];
    case QNetworkReply::InsecureRedirectError:
        [[fallthrough]];
    case QNetworkReply::ProxyConnectionRefusedError:
        [[fallthrough]];
    case QNetworkReply::ProxyConnectionClosedError:
        [[fallthrough]];
    case QNetworkReply::ProxyNotFoundError:
        [[fallthrough]];
    case QNetworkReply::ProxyTimeoutError:
        [[fallthrough]];
    case QNetworkReply::ProxyAuthenticationRequiredError:
        [[fallthrough]];
    case QNetworkReply::ServiceUnavailableError:
        alert = ConnectionFailedAlert;
        break;

    case QNetworkReply::HostNotFoundError:
        [[fallthrough]];
    case QNetworkReply::TimeoutError:
        [[fallthrough]];
    case QNetworkReply::UnknownNetworkError:
        // On mac, this means: no internet
        alert = NoConnectionAlert;
        break;

    case QNetworkReply::OperationCanceledError:
        [[fallthrough]];
    case QNetworkReply::BackgroundRequestNotAllowedError:
        [[fallthrough]];
    case QNetworkReply::ContentAccessDenied:
        [[fallthrough]];
    case QNetworkReply::ContentNotFoundError:
        [[fallthrough]];
    case QNetworkReply::ContentReSendError:
        [[fallthrough]];
    case QNetworkReply::ContentConflictError:
        [[fallthrough]];
    case QNetworkReply::ContentGoneError:
        [[fallthrough]];
    case QNetworkReply::InternalServerError:
        [[fallthrough]];
    case QNetworkReply::OperationNotImplementedError:
        [[fallthrough]];
    case QNetworkReply::ProtocolUnknownError:
        [[fallthrough]];
    case QNetworkReply::ProtocolInvalidOperationError:
        [[fallthrough]];
    case QNetworkReply::UnknownProxyError:
        [[fallthrough]];
    case QNetworkReply::UnknownContentError:
        [[fallthrough]];
    case QNetworkReply::ProtocolFailure:
        [[fallthrough]];
    case QNetworkReply::UnknownServerError:
        // let's ignore these errors.
        break;

    case QNetworkReply::ContentOperationNotPermittedError:
        [[fallthrough]];
    case QNetworkReply::AuthenticationRequiredError:
        alert = AuthenticationFailedAlert;
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
        m_settings.clear();
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
    m_serverData.writeSettings(m_settings);
}

void MozillaVPN::postAuthenticationCompleted()
{
    qDebug() << "Post authentication completed";
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
