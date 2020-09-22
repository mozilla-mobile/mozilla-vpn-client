#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QObject>

class QInAppStore;

class IAPHandler : public QObject
{
    Q_OBJECT

public:
    IAPHandler(QObject *parent) : QObject(parent) {}

    void start();

signals:
    void completed();

private:
    QInAppStore* m_appStore = nullptr;
};

#endif // IAPHANDLER_H
