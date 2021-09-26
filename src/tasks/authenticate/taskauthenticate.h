/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"
#include "core.h"

class QByteArray;
class AuthenticationListener;

class TaskAuthenticate final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskAuthenticate)

 public:
  explicit TaskAuthenticate(Core::AuthenticationType authenticationType);
  ~TaskAuthenticate();

  void run(Core* core) override;

 private:
  void authenticationCompleted(Core* core, const QByteArray& data);

 private:
  AuthenticationListener* m_authenticationListener = nullptr;
  Core::AuthenticationType m_authenticationType = Core::AuthenticationInBrowser;
};

#endif  // TASKAUTHENTICATE_H
