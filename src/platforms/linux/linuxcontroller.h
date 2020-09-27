#ifndef LINUXCONTROLLER_H
#define LINUXCONTROLLER_H

#include "controllerimpl.h"

class LinuxController final : public ControllerImpl
{
public:
    LinuxController() = default;

    void initialize(const Device *device, const Keys *keys) override;

    void activate(const Server &server,
                  const Device *device,
                  const Keys *keys,
                  bool forSwitching) override;

    void deactivate(bool forSwitching) override;

    void checkStatus() override;
};

#endif // LINUXCONTROLLER_H
