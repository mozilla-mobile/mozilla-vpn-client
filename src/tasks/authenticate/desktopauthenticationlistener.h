/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DESKTOPAUTHENTICATIONLISTENER_H
#define DESKTOPAUTHENTICATIONLISTENER_H

#include "authenticationlistener.h"

class QOAuthHttpServerReplyHandler;

class DesktopAuthenticationListener : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DesktopAuthenticationListener)

 public:
  explicit DesktopAuthenticationListener(QObject* parent,
                                         bool headless = false);
  ~DesktopAuthenticationListener();

  void start(Task* task, const QString& codeChallenge,
             const QString& codeChallengeMethod,
             const QString& emailAddress) override;

 private:
  QOAuthHttpServerReplyHandler* m_server = nullptr;
  const bool m_headless;
};

#endif  // DESKTOPAUTHENTICATIONLISTENER_H
