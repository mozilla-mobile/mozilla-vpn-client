#include "mozillavpn.h"
#include "taskauthenticate.h"

#include <QDebug>
#include <QPointer>

constexpr const char *STATE_INITIALIZE = "INITIALIZE";
constexpr const char *STATE_CONNECTING = "CONNECTING";

constexpr const char *API_URL_PROD = "https://fpn.firefox.com";
constexpr const char *API_URL_DEBUG = "https://fpn.firefox.com";

MozillaVPN::MozillaVPN(QObject *parent) : QObject(parent) {}

MozillaVPN::~MozillaVPN() = default;

void MozillaVPN::initialize(int &argc, char *argv[])
{
    qDebug() << "MozillaVPN Initialization";

    // TODO: read the config file
    m_state = STATE_INITIALIZE;

    m_apiUrl = API_URL_PROD;
    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "--debug")) {
            m_apiUrl = API_URL_DEBUG;
        }
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

    QObject::connect(task, SIGNAL(completed()), task, SLOT(deleteLater()));

    task->Run(this);
}
