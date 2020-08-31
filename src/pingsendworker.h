#ifndef PINGSENDWORKER_H
#define PINGSENDWORKER_H

#include <QObject>

class PingSendWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    virtual void sendPing(const QString &destination) = 0;
    virtual void stopPing() = 0;

signals:
    void pingSucceeded();
    void pingFailed();
};

#endif // PINGSENDWORKER_H
