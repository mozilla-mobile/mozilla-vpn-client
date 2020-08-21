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
#include <QPointer>
#include <QTimer>

constexpr const char *STATE_INITIALIZE = "INITIALIZE";
constexpr const char *STATE_CONNECTING = "CONNECTING";
constexpr const char *STATE_MAIN = "MAIN";

constexpr const char *API_URL_PROD = "https://fpn.firefox.com";
constexpr const char *API_URL_DEBUG = "https://stage.guardian.nonprod.cloudops.mozgcp.net";

constexpr const char *SETTINGS_TOKEN = "token";

// in seconds, how often we should fetch the server list.
constexpr const uint32_t SCHEDULE_SERVER_FETCH_TIMER_SEC = 3600;

// in seconds, how often we should check the account
constexpr const uint32_t SCHEDULE_ACCOUNT_CHECK_TIMER_SEC = 3600;

MozillaVPN::MozillaVPN(QObject *parent) : QObject(parent), m_settings("mozilla", "guardianvpn") {}

MozillaVPN::~MozillaVPN() = default;

void MozillaVPN::initialize(int &, char *[])
{
    qDebug() << "MozillaVPN Initialization";

    // This is our first state.
    m_state = STATE_INITIALIZE;

    // API URL depends on the type of build.
    m_apiUrl = API_URL_PROD;
#ifdef QT_DEBUG
    m_apiUrl = API_URL_DEBUG;
#endif

#ifdef QT_DEBUG
    //m_settings.clear();
#endif

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

    // Something went wrong during the previous authentications.
    if (!m_deviceModel.hasPrivateKeyDevice(Device::currentDeviceName())) {
        qDebug() << "No private key found for the current device";
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

    setState(STATE_MAIN);
}

void MozillaVPN::setState(const QString &state)
{
    m_state = state;
    emit stateChanged();
}

void MozillaVPN::authenticate()
{
    qDebug() << "Authenticate";

    m_state = STATE_CONNECTING;
    emit stateChanged();

    scheduleTask(new TaskAuthenticate());
}

void MozillaVPN::openLink(const QString &linkName)
{
    qDebug() << "Opening link: " << linkName;
    // TODO
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

void MozillaVPN::authenticationCompleted(QJsonObject &userObj, const QString &token)
{
    qDebug() << "Authentication completed";

    m_user.fromJson(userObj);
    m_user.writeSettings(m_settings);

    m_deviceModel.fromJson(userObj);
    m_deviceModel.writeSettings(m_settings);

    emit deviceModelChanged();

    m_settings.setValue(SETTINGS_TOKEN, token);
    m_token = token;

    qDebug() << "Maybe adding the device";

    QString deviceName = Device::currentDeviceName();

    if (m_deviceModel.hasPrivateKeyDevice(deviceName)) {
        deviceAdded(deviceName, QString(), QString());
        return;
    }

    // This device doesn't have a private key. Let's remove it.
    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    // Here we add the current device.
    scheduleTask(new TaskAddDevice(deviceName));

    // Then we fetch the list of servers.
    scheduleTask(new TaskFetchServers());

    // Finally we are able to activate the client.
    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        if (m_state == STATE_CONNECTING) {
            m_state = STATE_MAIN;
            emit stateChanged();
        }
    }));
}

void MozillaVPN::deviceAdded(const QString &deviceName,
                             const QString &publicKey,
                             const QString &privateKey)
{
    qDebug() << "Device added";

    if (!m_deviceModel.hasDevice(deviceName)) {
        m_deviceModel.addDevice(Device(deviceName, publicKey, privateKey));
        m_deviceModel.writeSettings(m_settings);
    }
}

void MozillaVPN::deviceRemoved(const QString &deviceName)
{
    qDebug() << "Device removed";

    m_deviceModel.removeDevice(deviceName);
    emit deviceModelChanged();
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

    emit serverCountryModelChanged();

    qDebug() << "Scheduling the server fetch";

    QTimer::singleShot(1000 * SCHEDULE_SERVER_FETCH_TIMER_SEC,
                       [this]() { scheduleTask(new TaskFetchServers()); });
}

void MozillaVPN::activate()
{
    qDebug() << "Activation";
}

void MozillaVPN::deactivate()
{
    qDebug() << "Deactivation";
}

int MozillaVPN::activeDevices() const
{
    // We need to expose "int"to make QML happy.
    return (int) m_deviceModel.count();
}

void MozillaVPN::removeDevice(const QString &deviceName)
{
    qDebug() << "Remove device" << deviceName;

    if (m_deviceModel.hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }
}

void MozillaVPN::accountChecked(QJsonObject &userObj)
{
    qDebug() << "Account checked";

    m_user.fromJson(userObj);
    m_user.writeSettings(m_settings);

    m_deviceModel.fromJson(userObj);
    m_deviceModel.writeSettings(m_settings);

    emit m_user.changed();

    qDebug() << "Scheduling the account check";
    QTimer::singleShot(1000 * SCHEDULE_ACCOUNT_CHECK_TIMER_SEC,
                       [this]() { scheduleTask(new TaskAccount()); });
}

void MozillaVPN::cancelAuthentication()
{
    qDebug() << "Canceling authentication";

    if (m_state != STATE_CONNECTING) {
        // We cannot cancel tasks if we are not in connecting state.
        return;
    }

    for (QList<QPointer<Task>>::Iterator i = m_tasks.begin(); i != m_tasks.end(); ++i) {
        delete *i;
    }

    m_task_running = false;
    m_tasks.clear();

    setState(STATE_INITIALIZE);
}

void MozillaVPN::logout()
{
    qDebug() << "Logout";

    QString deviceName = Device::currentDeviceName();

    if (m_deviceModel.hasPrivateKeyDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    scheduleTask(new TaskFunction([this](MozillaVPN *) {
        m_settings.clear();
        setState(STATE_INITIALIZE);

        m_alert = LogoutAlert;
        emit alertChanged();
    }));
}

void MozillaVPN::hideAlert() {
    m_alert = NoAlert;
    emit alertChanged();
}

void MozillaVPN::errorHandle(QNetworkReply::NetworkError error) {
    qDebug() << "Handling error" << error;

    Q_ASSERT(error != QNetworkReply::NoError);

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
        m_alert = ConnectionFailedAlert;
        break;

    case QNetworkReply::HostNotFoundError:
        [[fallthrough]];
    case QNetworkReply::TimeoutError:
        m_alert = NoConnectionAlert;
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
    case QNetworkReply::UnknownNetworkError:
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
       m_alert = AuthenticationFailedAlert;
       break;

    default:
        break;
    }
}
