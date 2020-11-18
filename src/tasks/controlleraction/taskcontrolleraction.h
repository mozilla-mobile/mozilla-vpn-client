/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKCONTROLLERACTION_H
#define TASKCONTROLLERACTION_H

#include "task.h"

class TaskControllerAction : public Task
{
public:
    enum TaskAction {
        eActivate,
        eDeactivate,
    };

    explicit TaskControllerAction(TaskAction action);

    void run(MozillaVPN *vpn) override;

private slots:
    void stateChanged();

private:
    const TaskAction m_action;
};

#endif // TASKCONTROLLERACTION_H
