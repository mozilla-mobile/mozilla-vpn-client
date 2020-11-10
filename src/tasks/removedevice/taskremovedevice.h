/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKREMOVEDEVICE_H
#define TASKREMOVEDEVICE_H

#include "task.h"

class TaskRemoveDevice final : public Task
{
public:
    explicit TaskRemoveDevice(const QString &deviceName);

    void run(MozillaVPN *vpn) override;

private:
    void maybeReset();

private:
    QString m_deviceName;
};

#endif // TASKREMOVEDEVICE_H
