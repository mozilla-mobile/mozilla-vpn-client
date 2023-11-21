/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKCONTROLLERACTION_H
#define TASKCONTROLLERACTION_H

#include <QObject>
#include <QTimer>

#include "controller.h"
#include "models/serverdata.h"
#include "task.h"

// The purpose of this task is to block any other task when
// activating/deactivating the VPN. It doesn't relay on the Controller state,
// but just wait a bit: 1 second is enough.
class TaskControllerAction final : public Task {
  Q_DISABLE_COPY_MOVE(TaskControllerAction)

 public:
  enum TaskAction {
    eActivate,
    eDeactivate,
    eSilentSwitch,
    eSwitch,
  };
  Q_ENUM(TaskAction);

  explicit TaskControllerAction(
      TaskAction action,
      Controller::ServerCoolDownPolicyForSilentSwitch serverCoolDownPolicy =
          Controller::eServerCoolDownNotNeeded);

  ~TaskControllerAction();

  void run() override;

  virtual DeletePolicy deletePolicy() const override { return NonDeletable; }

 private:
  void stateChanged();
  void checkStatus();

 private:
  const TaskAction m_action;
  Controller::State m_lastState;
  ServerData m_serverData;
  QTimer m_timer;
  Controller::ServerCoolDownPolicyForSilentSwitch m_serverCoolDownPolicy =
      Controller::eServerCoolDownNotNeeded;
};

#endif  // TASKCONTROLLERACTION_H
