/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidauthenticationlistener.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "platforms/android/androidutils.h"
#include "tasks/authenticate/desktopauthenticationlistener.h"

#include <QAndroidJniObject>
#include <QtAndroid>
#include <jni.h>

namespace {
Logger logger(LOG_ANDROID, "AndroidAuthenticationListener");
}

AndroidAuthenticationListener::AndroidAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MVPN_COUNT_CTOR(AndroidAuthenticationListener);
  logger.debug() << "Android authentication listener";
}

AndroidAuthenticationListener::~AndroidAuthenticationListener() {
  MVPN_COUNT_DTOR(AndroidAuthenticationListener);
}

void AndroidAuthenticationListener::start(const QString& codeChallenge,
                                          const QString& codeChallengeMethod,
                                          const QString& emailAddress) {
  logger.debug() << "Authenticationlistener initialize";

  QUrl url(createAuthenticationUrl(MozillaVPN::AuthenticationInBrowser,
                                   codeChallenge, codeChallengeMethod,
                                   emailAddress));

  QAndroidJniObject activity = QtAndroid::androidActivity();
  jboolean supported = QAndroidJniObject::callStaticMethod<jboolean>(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "isWebViewSupported",
      "(Landroid/content/Context;)Z", activity.object());
  if (supported) {
    AndroidUtils::instance()->startAuthentication(this, url);
    return;
  }
  DesktopAuthenticationListener* legacyAuth;
  legacyAuth = new DesktopAuthenticationListener(this);
  legacyAuth->start(codeChallenge, codeChallengeMethod, emailAddress);

  connect(legacyAuth, &AuthenticationListener::completed, this,
          &AndroidAuthenticationListener::completed);
  connect(legacyAuth, &AuthenticationListener::failed, this,
          &AndroidAuthenticationListener::failed);
}
