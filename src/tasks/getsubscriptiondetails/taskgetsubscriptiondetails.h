/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKGETSUBSCRIPTIONDETAILS_H
#define TASKGETSUBSCRIPTIONDETAILS_H

#include "errorhandler.h"
#include "taskscheduler/task.h"

class AuthenticationInAppSession;

class TaskGetSubscriptionDetails final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskGetSubscriptionDetails)

 public:
  enum AuthenticationPolicy {
    RunAuthenticationFlowIfNeeded,
    ForceAuthenticationFlow,
    NoAuthenticationFlow,
  };

  explicit TaskGetSubscriptionDetails(
      AuthenticationPolicy authenticationPolicy,
      ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy);
  ~TaskGetSubscriptionDetails();

  void run() override;

 signals:
  void operationCompleted(bool status);
  void needsAuthentication();

 private:
  void initAuthentication();
  void runInternal();
  void maybeComplete(bool status);

 private:
  AuthenticationInAppSession* m_authenticationInAppSession = nullptr;

  AuthenticationPolicy m_authenticationPolicy = RunAuthenticationFlowIfNeeded;
  ErrorHandler::ErrorPropagationPolicy m_errorPropagationPolicy =
      ErrorHandler::DoNotPropagateError;
};

#endif  // TASKGETSUBSCRIPTIONDETAILS_H
