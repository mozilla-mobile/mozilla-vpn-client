/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKCONTROLLERACTION_H
#define TASKCONTROLLERACTION_H

#include "task.h"

#include <QObject>
#include <QTimer>
#include "controller.h"

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
  };

  explicit TaskControllerAction(TaskAction action);
  ~TaskControllerAction();

  void run(MozillaVPN* vpn) override;

 private slots:
  void stateChanged();
  void silentSwitchDone();
  void checkStatus();

 private:
  const TaskAction m_action;
  Controller::State m_lastState;
  QTimer m_timer;
};

#endif  // TASKCONTROLLERACTION_H
