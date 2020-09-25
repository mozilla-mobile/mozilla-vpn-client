#ifndef MACOSCONTROLLER_H
#define MACOSCONTROLLER_H

#include "controllerimpl.h"

class MacOSController final : public ControllerImpl
{
public:
    void initialize(const Device *device, const Keys *keys) override;

    void activate(const Server &data,
                  const Device *device,
                  const Keys *keys,
                  bool forSwitching) override;

    void deactivate(const Server &server,
                    const Device *device,
                    const Keys *keys,
                    bool forSwitching) override;

    void checkStatus() override;

private:
    bool m_checkingStatus = false;
};

#endif // MACOSCONTROLLER_H
