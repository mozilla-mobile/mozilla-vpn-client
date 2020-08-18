#include "mozillavpn.h"
#include "serverdata.h"
#include "taskadddevice.h"
#include "taskauthenticate.h"
#include "taskfetchservers.h"
#include "taskremovedevice.h"
#include "userdata.h"

#include <QDebug>
#include <QPointer>

constexpr const char *STATE_INITIALIZE = "INITIALIZE";
constexpr const char *STATE_CONNECTING = "CONNECTING";
constexpr const char *STATE_OFF = "OFF";

constexpr const char *API_URL_PROD = "https://fpn.firefox.com";
constexpr const char *API_URL_DEBUG = "https://stage.guardian.nonprod.cloudops.mozgcp.net";

constexpr const char *SETTINGS_TOKEN = "token";

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
    UserData *userData = UserData::fromSettings(m_settings);
    Q_ASSERT(userData);

    // Something went wrong during the previous authentications.
    if (!userData->hasPrivateKeyDevice(DeviceData::currentDeviceName())) {
        qDebug() << "No private key found for the current device";

        m_settings.clear();
        delete userData;
        return;
    }

    ServerData *servers = ServerData::fromSettings(m_settings);
    if (!servers) {
        qDebug() << "No server list found";

        m_settings.clear();
        delete userData;
        return;
    }

    m_token = m_settings.value(SETTINGS_TOKEN).toString();

    Q_ASSERT(!m_userData);
    m_userData = userData;

    Q_ASSERT(!m_servers);
    m_servers = servers;

    // TODO: what's the right state here?
    m_state = STATE_OFF;
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

void MozillaVPN::authenticationCompleted(UserData *userData, const QString &token)
{
    qDebug() << "Authentication completed";

    Q_ASSERT(!m_userData);
    m_userData = userData;
    m_userData->writeSettings(m_settings);

    m_settings.setValue(SETTINGS_TOKEN, token);
    m_token = token;

    qDebug() << "Maybe adding the device";

    QString deviceName = DeviceData::currentDeviceName();

    if (m_userData->hasPrivateKeyDevice(deviceName)) {
        deviceAdded(deviceName, QString(), QString());
        return;
    }

    // This device doesn't have a private key. Let's remove it.
    if (m_userData->hasDevice(deviceName)) {
        scheduleTask(new TaskRemoveDevice(deviceName));
    }

    scheduleTask(new TaskAddDevice(deviceName));
}

void MozillaVPN::deviceAdded(const QString &deviceName,
                             const QString &publicKey,
                             const QString &privateKey)
{
    qDebug() << "Device added";

    Q_ASSERT(m_userData);
    if (!m_userData->hasDevice(deviceName)) {
        m_userData->addDevice(DeviceData(deviceName, publicKey, privateKey));
        m_userData->writeSettings(m_settings);
    }

    scheduleTask(new TaskFetchServers());
}

void MozillaVPN::deviceRemoved(const QString &deviceName)
{
    qDebug() << "Device removed";

    Q_ASSERT(m_userData);
    m_userData->removeDevice(deviceName);
}

void MozillaVPN::serversFetched(ServerData *servers)
{
    qDebug() << "Server fetched!";

    if (m_servers) {
        delete m_servers;
    }

    Q_ASSERT(servers);
    m_servers = servers;
    m_servers->writeSettings(m_settings);

    m_state = STATE_OFF;
    emit stateChanged();
}

void MozillaVPN::activate()
{
    qDebug() << "Activation";
}
