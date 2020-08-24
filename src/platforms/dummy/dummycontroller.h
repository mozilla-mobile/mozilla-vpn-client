#ifndef DUMMYCONTROLLER_H
#define DUMMYCONTROLLER_H

#include "controllerimpl.h"

class QTimer;
class Server;

class DummyController final : public ControllerImpl
{
public:
    DummyController();

    void activate(const Server &data) override;

    void deactivate() override;

private:
    QTimer* m_timer;
};

#endif // DUMMYCONTROLLER_H
