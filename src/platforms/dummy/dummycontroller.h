#ifndef DUMMYCONTROLLER_H
#define DUMMYCONTROLLER_H

#include "controllerimpl.h"

class DummyController final : public ControllerImpl
{
public:
    void activate(const Server &data,
                  const Device *device,
                  const Keys *keys,
                  bool forSwitching) override;

    void deactivate(const Server &server,
                    const Device *device,
                    const Keys *keys,
                    bool forSwitching) override;
};

#endif // DUMMYCONTROLLER_H
