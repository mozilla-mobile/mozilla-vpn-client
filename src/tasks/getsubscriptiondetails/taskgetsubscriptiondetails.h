/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKGETSUBSCRIPTIONDETAILS_H
#define TASKGETSUBSCRIPTIONDETAILS_H

#include "task.h"

class AuthenticationInAppSession;

class TaskGetSubscriptionDetails final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskGetSubscriptionDetails)

 public:
  explicit TaskGetSubscriptionDetails(const QString& emailAddress,
                                      const bool forceReauth);
  ~TaskGetSubscriptionDetails();

  void run() override;

 signals:
  void receivedData(const QByteArray& data);
  void failed();
  void needsAuthentication();

 private:
  void initAuthentication();

 private:
  AuthenticationInAppSession* m_authenticationInAppSession = nullptr;
  const QString m_emailAddress;
  bool m_forceReauth;
};

#endif  // TASKGETSUBSCRIPTIONDETAILS_H
