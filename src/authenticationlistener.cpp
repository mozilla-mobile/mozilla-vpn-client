/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationlistener.h"
#include "authenticationinapp/authenticationinapplistener.h"
#include "leakdetector.h"
#include "logger.h"

#if defined(MVPN_ANDROID)
#  include "platforms/android/androidauthenticationlistener.h"
#elif defined(MVPN_IOS)
#  include "platforms/ios/iosauthenticationlistener.h"
#elif defined(MVPN_WASM)
#  include "platforms/wasm/wasmauthenticationlistener.h"
#else
#  include "tasks/authenticate/desktopauthenticationlistener.h"
#endif

namespace {
Logger logger(LOG_MAIN, "AuthenticationListener");
}  // anonymous namespace

// static
AuthenticationListener* AuthenticationListener::create(
    QObject* parent, MozillaVPN::AuthenticationType authenticationType) {
  switch (authenticationType) {
    case MozillaVPN::AuthenticationInBrowser:
#if defined(MVPN_ANDROID)
      return new AndroidAuthenticationListener(parent);
#elif defined(MVPN_IOS)
      return new IOSAuthenticationListener(parent);
#elif defined(MVPN_WASM)
      return new WasmAuthenticationListener(parent);
#else
      return new DesktopAuthenticationListener(parent);
#endif
    case MozillaVPN::AuthenticationInApp:
      return new AuthenticationInAppListener(parent);

    case MozillaVPN::DefaultAuthentication:
    default:
      Q_ASSERT(false);
  }
}

AuthenticationListener::AuthenticationListener(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(AuthenticationListener);
}

AuthenticationListener::~AuthenticationListener() {
  MVPN_COUNT_DTOR(AuthenticationListener);
}
