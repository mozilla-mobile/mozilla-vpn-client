/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationlistener.h"

#include "appconstants.h"
#include "authenticationinapp/authenticationinapplistener.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkmanager.h"

#if defined(MZ_MACOS)
#  include "platforms/macos/macosauthenticationlistener.h"
#elif defined(MZ_WASM)
#  include "platforms/wasm/wasmauthenticationlistener.h"
#else
#  include "tasks/authenticate/desktopauthenticationlistener.h"
#endif

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QUrlQuery>

namespace {
Logger logger("AuthenticationListener");
}  // anonymous namespace

// static
AuthenticationListener* AuthenticationListener::create(
    QObject* parent, MozillaVPN::AuthenticationType authenticationType) {
  switch (authenticationType) {
    case MozillaVPN::AuthenticationInBrowser:
#if defined(MZ_ANDROID) or defined(MZ_IOS)
      logger.error() << "Something went totally wrong";
      Q_ASSERT(false);
#elif defined(MZ_MACOS)
      return new MacosAuthenticationListener(parent);
#elif defined(MZ_WASM)
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
  MZ_COUNT_CTOR(AuthenticationListener);
}

AuthenticationListener::~AuthenticationListener() {
  MZ_COUNT_DTOR(AuthenticationListener);
}

// static
QUrl AuthenticationListener::createAuthenticationUrl(
    const QString& codeChallenge, const QString& codeChallengeMethod,
    const QString& emailAddress) {
  QString path("/api/v2/vpn/login/");

#if !defined(MZ_DUMMY)
  path.append(Constants::PLATFORM_NAME);
#else
  // Let's use ios here.
  path.append("ios");
#endif

  QUrl url(AppConstants::apiBaseUrl());
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

void AuthenticationListener::aboutToFinish() { emit readyToFinish(); }

// static
void AuthenticationListener::generatePkceCodes(QByteArray& pkceCodeVerifier,
                                               QByteArray& pkceCodeChallenge) {
  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  pkceCodeVerifier.clear();
  static QByteArray range(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  for (uint16_t i = 0; i < 128; ++i) {
    pkceCodeVerifier.append(range.at(generator->generate() % range.length()));
  }

  pkceCodeChallenge =
      QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Sha256)
          .toBase64(QByteArray::Base64UrlEncoding);
  Q_ASSERT(pkceCodeChallenge.length() == 44);
}
