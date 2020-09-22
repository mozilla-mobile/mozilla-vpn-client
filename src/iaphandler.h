#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QObject>

class IAPHandler : public QObject
{
    Q_OBJECT

public:
    IAPHandler(QObject *parent) : QObject(parent) {}

    void start();

signals:
    void completed();
};

#endif // IAPHANDLER_H
