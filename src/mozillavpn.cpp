#include "mozillavpn.h"
#include "taskadddevice.h"
#include "taskauthenticate.h"
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

void MozillaVPN::initialize(int &argc, char *argv[])
{
    qDebug() << "MozillaVPN Initialization";

    // This is our first state.
    m_state = STATE_INITIALIZE;

    // API URL depends on the type of build.
    m_apiUrl = API_URL_PROD;
#ifdef QT_DEBUG
    m_apiUrl = API_URL_DEBUG;
#endif
    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "--debug")) {
            m_apiUrl = API_URL_DEBUG;
        }
    }

#ifdef QT_DEBUG
    m_settings.clear();
#endif

    if (m_settings.contains(SETTINGS_TOKEN)) {
        qDebug() << "We have a valid token";

        m_token = m_settings.value(SETTINGS_TOKEN).toString();
        m_userData = UserData::fromSettings(m_settings);
        Q_ASSERT(m_userData);

        // TODO: what about if the device doesn't exist yet...?
        // TODO: what's the right state here?
        m_state = STATE_OFF;
    }
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

    QString deviceName = QSysInfo::machineHostName() + " " + QSysInfo::productType() + " "
                         + QSysInfo::productVersion();
    if (m_userData->hasDevice(deviceName)) {
        m_state = STATE_OFF;
        emit stateChanged();
        return;
    }

    scheduleTask(new TaskAddDevice(deviceName));
}

void MozillaVPN::deviceAdded(const QString &deviceName, const QString &publicKey)
{
    qDebug() << "Device added";

    Q_ASSERT(m_userData);
    m_userData->addDevice(DeviceData(deviceName, publicKey));

    // TODO: do we need to add the device to the settings/userData ?
    m_state = STATE_OFF;
    emit stateChanged();
}
