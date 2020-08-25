#ifndef CONTROLLERIMPL_H
#define CONTROLLERIMPL_H

#include <QObject>

class Keys;
class Device;
class Server;

class ControllerImpl : public QObject
{
    Q_OBJECT

public:
    ControllerImpl() = default;

    virtual ~ControllerImpl() = default;

    virtual void activate(const Server &server, const Device* device, const Keys* keys) = 0;
    virtual void deactivate() = 0;

signals:
    void connected();
    void disconnected();
};

#endif // CONTROLLERIMPL_H
