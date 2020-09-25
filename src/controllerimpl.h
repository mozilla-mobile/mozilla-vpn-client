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

    virtual void initialize(const Device *device, const Keys *keys)
    {
        Q_UNUSED(device);
        Q_UNUSED(keys);

        emit initialized(true, Controller::StateOff, QDateTime());
    }

    virtual void activate(const Server &server,
                          const Device *device,
                          const Keys *keys,
                          bool forSwitching)
        = 0;
    virtual void deactivate(const Server &server,
                            const Device *device,
                            const Keys *keys,
                            bool forSwitching)
        = 0;

    virtual void checkStatus() {}

signals:
    void initialized(bool status, Controller::State state, const QDateTime& connectionDate);

    void connected();
    void disconnected();

    void statusUpdated(/* TODO */);
};

#endif // CONTROLLERIMPL_H
