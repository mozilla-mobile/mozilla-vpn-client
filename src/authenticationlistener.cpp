/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationlistener.h"
#include "authenticationinapp/authenticationinapplistener.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkmanager.h"
#include "networkrequest.h"

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

    default:
      Q_ASSERT(false);
      return nullptr;
  }
}

AuthenticationListener::AuthenticationListener(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(AuthenticationListener);
}

AuthenticationListener::~AuthenticationListener() {
  MVPN_COUNT_DTOR(AuthenticationListener);
}

// static
QUrl AuthenticationListener::createAuthenticationUrl(
    MozillaVPN::AuthenticationType authenticationType,
    const QString& codeChallenge, const QString& codeChallengeMethod,
    const QString& emailAddress) {
  QString path("/api/v2/vpn/login/");

  if (authenticationType == MozillaVPN::AuthenticationInApp) {
    // hack!
    path.append("android");
  } else {
    Q_ASSERT(authenticationType == MozillaVPN::AuthenticationInBrowser);
#if !defined(MVPN_DUMMY)
    path.append(Constants::PLATFORM_NAME);
#else
    // Let's use linux here.
    path.append("linux");
#endif
  }

  QUrl url(NetworkRequest::apiBaseUrl());
  url.setPath(path);

  QUrlQuery query;
  query.addQueryItem("code_challenge", codeChallenge);
  query.addQueryItem("code_challenge_method", codeChallengeMethod);
  query.addQueryItem("user_agent", NetworkManager::userAgent());

  if (!emailAddress.isEmpty()) {
    query.addQueryItem("email", emailAddress);
  }

  url.setQuery(query);
  return url;
}
