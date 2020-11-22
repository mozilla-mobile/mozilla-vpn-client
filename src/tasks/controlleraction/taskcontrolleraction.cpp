/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcontrolleraction.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

namespace {
Logger logger(QStringList{LOG_MAIN, LOG_CONTROLLER}, "TaskControllerAction");
}

TaskControllerAction::TaskControllerAction(TaskControllerAction::TaskAction action)
    : Task("TaskControllerAction"), m_action(action)
{
    MVPN_COUNT_CTOR(TaskControllerAction);

    logger.log() << "TaskControllerAction created for"
                 << (action == eActivate ? "activation" : "deactivation");

    connect(&m_timer, &QTimer::timeout, this, &TaskControllerAction::completed);
}

TaskControllerAction::~TaskControllerAction()
{
    MVPN_COUNT_DTOR(TaskControllerAction);
}

void TaskControllerAction::run(MozillaVPN *vpn)
{
    logger.log() << "TaskControllerAction run";

    switch (m_action) {
    case eActivate:
        vpn->controller()->activate();
        break;

    case eDeactivate:
        vpn->controller()->deactivate();
        break;
    }

    m_timer.start();
}
