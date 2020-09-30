/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H

#include "controllerimpl.h"

class AndroidController final : public ControllerImpl
{
public:
    void initialize(const Device *device, const Keys *keys) override
    {
        Q_UNUSED(device);
        Q_UNUSED(keys);

        emit initialized(true, Controller::StateOff, QDateTime());
    }

    void activate(const Server &data,
                  const Device *device,
                  const Keys *keys,
                  bool forSwitching) override;

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

private:
    int64_t m_txBytes = 0;
    int64_t m_rxBytes = 0;
};

#endif // DUMMYCONTROLLER_H
