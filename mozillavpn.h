#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include <memory>
#include <vector>
#include <QObject>

class Task;

class MozillaVPN final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    void initialize();

    QString getState() const { return m_state; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void openLink(const QString &linkName);

private:
    void scheduleTask(std::unique_ptr<Task> task);
    void maybeRunTask();

signals:
    void stateChanged();

private:
    std::vector<std::unique_ptr<Task>> m_tasks;
    bool m_task_running = false;

    QString m_state;
};

#endif // MOZILLAVPN_H
