/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONINAPPLISTENER_H
#define AUTHENTICATIONINAPPLISTENER_H

#include "authenticationlistener.h"

class AuthenticationInAppSession;

class AuthenticationInAppListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationInAppListener)

 public:
  explicit AuthenticationInAppListener(QObject* parent);
  ~AuthenticationInAppListener();

  void start(Task* task, const QString& codeChallenge,
             const QString& codeChallengeMethod,
             const QString& emailAddress) override;

  void aboutToFinish() override;

 private:
  void fallbackRequired();

 private:
  AuthenticationInAppSession* m_session = nullptr;
  Task* m_task = nullptr;

  QString m_codeChallenge;
  QString m_codeChallengeMethod;
};

#endif  // AUTHENTICATIONINAPPLISTENER_H
