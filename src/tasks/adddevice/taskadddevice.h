/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKADDDEVICE_H
#define TASKADDDEVICE_H

#include "task.h"

class TaskAddDevice final : public Task
{
public:
    explicit TaskAddDevice(const QString &deviceName);

    void run(MozillaVPN *vpn) override;

private:
    QString m_deviceName;
};

#endif // TASKADDDEVICE_H
