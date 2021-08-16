/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMAUTHENTICATIONLISTENER_H
#define WASMAUTHENTICATIONLISTENER_H

#include "authenticationlistener.h"

class WasmAuthenticationListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WasmAuthenticationListener)

 public:
  explicit WasmAuthenticationListener(QObject* parent);
  ~WasmAuthenticationListener();

  void start(const QString& codeChallenge, const QString& codeChallengeMethod,
             const QString& emailAddress) override;
};

#endif  // WASMAUTHENTICATIONLISTENER_H
