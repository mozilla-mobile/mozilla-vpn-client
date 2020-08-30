#ifndef PINGSENDER_H
#define PINGSENDER_H

#include <QObject>

class PingSender : public QObject
{
    Q_OBJECT

public:
    explicit PingSender(QObject *parent) : QObject(parent) {}

    virtual void send(const QString &gateway) = 0;

    virtual void stop() = 0;

signals:
    void completed();
};

#endif // PINGSENDER_H
