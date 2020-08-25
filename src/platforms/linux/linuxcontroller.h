#ifndef LINUXCONTROLLER_H
#define LINUXCONTROLLER_H

#include "controllerimpl.h"

class LinuxController final : public ControllerImpl
{
public:
    LinuxController() = default;

    void activate(const Server &server, const Device *device, const Keys *keys) override;

    void deactivate(const Server &server, const Device *device, const Keys *keys) override;
};

#endif // LINUXCONTROLLER_H
