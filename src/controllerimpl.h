#ifndef CONTROLLERIMPL_H
#define CONTROLLERIMPL_H

#include <QObject>

class ControllerImpl : public QObject
{
    Q_OBJECT

public:
    ControllerImpl() = default;

    virtual ~ControllerImpl() = default;

    virtual void activate() = 0;
    virtual void deactivate() = 0;

signals:
    void connected();
    void disconnected();
};

#endif // CONTROLLERIMPL_H
