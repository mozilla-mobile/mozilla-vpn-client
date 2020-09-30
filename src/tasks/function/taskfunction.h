/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKFUNCTION_H
#define TASKFUNCTION_H

#include <functional>

#include "task.h"

class TaskFunction final : public Task
{
public:
    TaskFunction(std::function<void(MozillaVPN *)> &&callback)
        : Task("TaskFunction"), m_callback(std::move(callback))
    {}

    void run(MozillaVPN *vpn) override
    {
        m_callback(vpn);
        emit completed();
    }

private:
    std::function<void(MozillaVPN *)> m_callback;
};

#endif // TASKFUNCTION_H
