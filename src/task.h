#ifndef TASK_H
#define TASK_H

#include <QObject>

class MozillaVPN;

class Task : public QObject
{
    Q_OBJECT

public:
    explicit Task(const QString &name) : m_name(name) {}
    virtual ~Task() = default;

    const QString &name() const { return m_name; }

    virtual void Run(MozillaVPN* aVPN) = 0;

signals:
    void completed();

private:
    QString m_name;
};

#endif // TASK_H
