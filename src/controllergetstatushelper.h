#ifndef CONTROLLERGETSTATUSHELPER_H
#define CONTROLLERGETSTATUSHELPER_H

#include <QObject>

class ControllerGetStatusHelper : public QObject
{
    Q_OBJECT
public:
    explicit ControllerGetStatusHelper(
        QObject *parent,
        std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>
            &&callback);

public Q_SLOTS:
    void statusUpdated(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes);

private:
    std::function<void(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes)>
        m_callback;
};

#endif // CONTROLLERGETSTATUSHELPER_H
