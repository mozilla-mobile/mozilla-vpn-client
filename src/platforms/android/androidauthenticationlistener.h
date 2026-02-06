/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDAUTHENTICATIONLISTENER_H
#define ANDROIDAUTHENTICATIONLISTENER_H

#include "authenticationlistener.h"

class AndroidAuthenticationListener final : public AuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AndroidAuthenticationListener)

 public:
  explicit AndroidAuthenticationListener(QObject* parent);
  ~AndroidAuthenticationListener();

  void start(Task* task, const QString& codeChallenge,
             const QString& codeChallengeMethod,
             const QString& emailAddress) override;

 private:
  static constexpr const char* CUSTOM_TAB_HELPER_CLASS =
      "org/mozilla/firefox/vpn/qt/CustomTabHelper";
};

#endif  // ANDROIDAUTHENTICATIONLISTENER_H
