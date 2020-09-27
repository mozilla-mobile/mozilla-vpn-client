#ifndef CONTROLLERIMPL_H
#define CONTROLLERIMPL_H

#include "controller.h"

#include <QObject>

class Keys;
class Device;
class Server;
class QDateTime;

class ControllerImpl : public QObject
{
    Q_OBJECT

public:
    ControllerImpl() = default;

    virtual ~ControllerImpl() = default;

    virtual void initialize(const Device *device, const Keys *keys) = 0;

    virtual void activate(const Server &server,
                          const Device *device,
                          const Keys *keys,
                          bool forSwitching)
        = 0;

    virtual void deactivate(bool forSwitching) = 0;

    virtual void checkStatus() = 0;

signals:
    void initialized(bool status, Controller::State state, const QDateTime &connectionDate);

    void connected();
    void disconnected();

    void statusUpdated(const QString &serverIpv4Gateway, uint64_t txBytes, uint64_t rxBytes);
};

#endif // CONTROLLERIMPL_H
