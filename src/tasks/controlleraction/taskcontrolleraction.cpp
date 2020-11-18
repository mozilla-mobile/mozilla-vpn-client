/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcontrolleraction.h"
#include "logger.h"
#include "mozillavpn.h"

namespace {
Logger logger(QStringList{LOG_MAIN, LOG_CONTROLLER}, "TaskControllerAction");
}

TaskControllerAction::TaskControllerAction(TaskControllerAction::TaskAction action)
    : Task("TaskControllerAction"), m_action(action)
{
    logger.log() << "TaskControllerAction created for"
                 << (action == eActivate ? "activation" : "deactivation");
}

void TaskControllerAction::run(MozillaVPN *vpn)
{
    logger.log() << "TaskControllerAction run";

    connect(vpn->controller(), &Controller::stateChanged, this, &TaskControllerAction::stateChanged);
    switch (m_action) {
    case eActivate:
        vpn->controller()->activate();
        break;

    case eDeactivate:
        vpn->controller()->deactivate();
        break;
    }
}

void TaskControllerAction::stateChanged()
{
    Controller::State state = MozillaVPN::instance()->controller()->state();
    logger.log() << "state changed received:" << state;

    if (state == Controller::StateOn || state == Controller::StateOff) {
        emit completed();
    }
}
