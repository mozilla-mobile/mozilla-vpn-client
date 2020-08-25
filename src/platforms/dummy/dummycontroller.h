#ifndef DUMMYCONTROLLER_H
#define DUMMYCONTROLLER_H

#include "controllerimpl.h"

class QTimer;

class DummyController final : public ControllerImpl
{
public:
    DummyController();

    void activate(const Server &data, const Device* device, const Keys* keys) override;

    void deactivate() override;

private:
    QTimer* m_timer;
};

#endif // DUMMYCONTROLLER_H
