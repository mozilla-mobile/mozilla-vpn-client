/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKDELETEACCOUNT_H
#define TASKDELETEACCOUNT_H

#include "taskscheduler/task.h"

class AuthenticationInAppSession;

class TaskDeleteAccount final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskDeleteAccount)

 public:
  explicit TaskDeleteAccount(const QString& emailAddress);
  ~TaskDeleteAccount();

  void run() override;

 signals:
  void accountDeleted();

 private:
  AuthenticationInAppSession* m_authenticationInAppSession = nullptr;
  const QString m_emailAddress;
};

#endif  // TASKDELETEACCOUNT_H
