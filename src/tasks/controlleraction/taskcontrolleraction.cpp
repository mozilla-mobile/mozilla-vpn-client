/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcontrolleraction.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

constexpr uint32_t TASKCONTROLLER_TIMER_MSEC = 3000;

namespace {
Logger logger(QStringList{LOG_MAIN, LOG_CONTROLLER}, "TaskControllerAction");
}

TaskControllerAction::TaskControllerAction(
    TaskControllerAction::TaskAction action)
    : Task("TaskControllerAction"),
      m_action(action),
      m_lastState(Controller::State::StateOff) {
  MVPN_COUNT_CTOR(TaskControllerAction);

  logger.debug() << "TaskControllerAction created for"
                 << (action == eActivate ? "activation" : "deactivation");

  connect(&m_timer, &QTimer::timeout, this, &TaskControllerAction::checkStatus);
}

TaskControllerAction::~TaskControllerAction() {
  MVPN_COUNT_DTOR(TaskControllerAction);
}

void TaskControllerAction::run(MozillaVPN* vpn) {
  logger.debug() << "TaskControllerAction run";

  Controller* controller = vpn->controller();
  Q_ASSERT(controller);

  if (m_action == eSilentSwitch) {
    connect(controller, &Controller::silentSwitchDone, this,
            &TaskControllerAction::silentSwitchDone);
  } else {
    connect(controller, &Controller::stateChanged, this,
            &TaskControllerAction::stateChanged);
  }

  bool expectSignal = false;

  m_lastState = controller->state();

  switch (m_action) {
    case eActivate:
      expectSignal = controller->activate();
      break;

    case eDeactivate:
      expectSignal = controller->deactivate();
      break;

    case eSilentSwitch:
      expectSignal = controller->silentSwitchServers();
      break;
  }

  // No signal expected. Probably, the VPN is already in the right state. Let's
  // use the timer to wait 1 cycle only.
  if (!expectSignal) {
    m_timer.start(0);
    return;
  }

  // Fallback 3 seconds.
  m_timer.start(TASKCONTROLLER_TIMER_MSEC);
}

void TaskControllerAction::stateChanged() {
  if (!m_timer.isActive()) {
    logger.debug() << "stateChanged received by to be ignored";
    return;
  }

  Controller* controller = MozillaVPN::instance()->controller();
  Q_ASSERT(controller);

  Controller::State state = controller->state();
  if ((m_action == eActivate && state == Controller::StateOn) ||
      (m_action == eDeactivate && state == Controller::StateOff)) {
    logger.debug() << "Operation completed";
    m_timer.stop();
    emit completed();
  }
}

void TaskControllerAction::silentSwitchDone() {
  logger.debug() << "Operation completed";
  m_timer.stop();
  emit completed();
}

void TaskControllerAction::checkStatus() {
  Controller* controller = MozillaVPN::instance()->controller();
  Q_ASSERT(controller);
  if (controller->state() == m_lastState) {
    m_timer.stop();
    emit completed();
  } else {
    m_lastState = controller->state();
  }
}