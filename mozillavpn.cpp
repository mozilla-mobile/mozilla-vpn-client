#include "mozillavpn.h"
#include "taskauthenticate.h"

#include <QDebug>
#include <QPointer>

constexpr const char *STATE_INITIALIZE = "INITIALIZE";
constexpr const char *STATE_CONNECTING = "CONNECTING";

MozillaVPN::MozillaVPN(QObject *parent) : QObject(parent) {}

MozillaVPN::~MozillaVPN() = default;

void MozillaVPN::initialize()
{
    qDebug() << "MozillaVPN Initialization";

    // TODO: read the config file
    m_state = STATE_INITIALIZE;
}

void MozillaVPN::authenticate()
{
    qDebug() << "Authenticate";

    m_state = STATE_CONNECTING;
    emit stateChanged();

    std::unique_ptr<Task> task(new TaskAuthenticate());
    scheduleTask(std::move(task));
}

void MozillaVPN::openLink(const QString &linkName)
{
    qDebug() << "Opening link: " << linkName;
    // TODO
}

void MozillaVPN::scheduleTask(std::unique_ptr<Task> task)
{
    qDebug() << "Scheduling task: " << task->name();

    m_tasks.push_back(std::move(task));
    maybeRunTask();
}

void MozillaVPN::maybeRunTask()
{
    qDebug() << "Tasks: " << m_tasks.size();

    if (m_task_running || m_tasks.empty()) {
        return;
    }

    m_task_running = true;
    std::unique_ptr<Task> &task = m_tasks.front();

    QPointer<MozillaVPN> self = this;
    QObject::connect(task.get(), &Task::completed, this, [self]() {
        qDebug() << "Task completed";

        self->m_tasks.front()->disconnect();
        self->m_tasks.erase(self->m_tasks.begin());

        self->m_task_running = false;
        self->maybeRunTask();
    });

    task->Run();
}
