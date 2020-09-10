#ifndef MACOSCONTROLLER_H
#define MACOSCONTROLLER_H

#include "controllerimpl.h"

class MacOSController final : public ControllerImpl
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

#endif // MACOSCONTROLLER_H
