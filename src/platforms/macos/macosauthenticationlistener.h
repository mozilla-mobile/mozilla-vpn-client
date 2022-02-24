/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSAUTHENTICATIONLISTENER_H
#define MACOSAUTHENTICATIONLISTENER_H

#include "authenticationlistener.h"

#include <QEvent>

class MacosAuthenticationListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MacosAuthenticationListener)

 public:
  explicit MacosAuthenticationListener(QObject* parent);
  ~MacosAuthenticationListener();

  void start(Task* task, const QString& codeChallenge,
             const QString& codeChallengeMethod,
             const QString& emailAddress) override;

 private:
  bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif  // MACOSAUTHENTICATIONLISTENER_H
