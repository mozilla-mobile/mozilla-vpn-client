/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKAUTHENTICATE_H
#define TASKAUTHENTICATE_H

#include "task.h"
#include "mozillavpn.h"

class QByteArray;
class AuthenticationListener;

class TaskAuthenticate final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TaskAuthenticate)

 public:
  explicit TaskAuthenticate(MozillaVPN::AuthenticationType authenticationType);
  ~TaskAuthenticate();

  void run(MozillaVPN* vpn) override;

 private:
  void authenticationCompleted(MozillaVPN* vpn, const QByteArray& data);

 private:
  AuthenticationListener* m_authenticationListener = nullptr;
  MozillaVPN::AuthenticationType m_authenticationType =
      MozillaVPN::AuthenticationInBrowser;
};

#endif  // TASKAUTHENTICATE_H
