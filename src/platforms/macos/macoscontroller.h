/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

private:
    bool m_checkingStatus = false;
};

#endif // MACOSCONTROLLER_H
