/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKACCOUNTANDSERVERS_H
#define TASKACCOUNTANDSERVERS_H

#include "task.h"

#include <QObject>

class TaskAccountAndServers final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAccountAndServers)

 public:
  TaskAccountAndServers();
  ~TaskAccountAndServers();

  void run(MozillaVPN* vpn) override;

 private:
  void maybeCompleted();

 private:
  QByteArray m_serverData;
  QByteArray m_serverExtraData;

  bool m_accountCompleted = false;
  bool m_serversCompleted = false;
  bool m_serverExtraCompleted = false;
};

#endif  // TASKACCOUNTANDSERVERS_H
