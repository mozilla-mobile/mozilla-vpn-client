/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKACCOUNTANDSERVERS_H
#define TASKACCOUNTANDSERVERS_H

#include "task.h"

#include <QObject>
#include <QPointer>

class TaskAccountAndServers final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAccountAndServers)

 public:
  TaskAccountAndServers();
  ~TaskAccountAndServers();

  void run(MozillaVPN* vpn) override;

 private:
  void maybeCompleted();

 private:
  bool m_accountCompleted = false;
  bool m_serversCompleted = false;
};

#endif  // TASKACCOUNTANDSERVERS_H
