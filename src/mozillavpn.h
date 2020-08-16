#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include <QList>
#include <QObject>
#include <QPointer>

class Task;

class MozillaVPN final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)

public:
    explicit MozillaVPN(QObject *parent = nullptr);
    ~MozillaVPN();

    void initialize(int &argc, char *argv[]);

    QString getState() const { return m_state; }

    const QString& getApiUrl() const { return m_apiUrl; }

    Q_INVOKABLE void authenticate();

    Q_INVOKABLE void openLink(const QString &linkName);

private:
    void scheduleTask(Task* task);
    void maybeRunTask();

signals:
    void stateChanged();

private:
    QList<QPointer<Task>> m_tasks;
    bool m_task_running = false;

    QString m_state;
    QString m_apiUrl;
};

#endif // MOZILLAVPN_H
