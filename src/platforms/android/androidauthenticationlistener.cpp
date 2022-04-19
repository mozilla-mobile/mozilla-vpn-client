/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidauthenticationlistener.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "platforms/android/androidutils.h"
#include "tasks/authenticate/desktopauthenticationlistener.h"
#include <jni.h>
#include <QJniObject>
#include <QJniEnvironment>

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

void AndroidAuthenticationListener::start(Task* task,
                                          const QString& codeChallenge,
                                          const QString& codeChallengeMethod,
                                          const QString& emailAddress) {
  logger.debug() << "Authenticationlistener initialize";

  QUrl url(createAuthenticationUrl(codeChallenge, codeChallengeMethod,
                                   emailAddress));

  QJniObject activity = AndroidUtils::getActivity();
  jboolean supported = QJniObject::callStaticMethod<jboolean>(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "isWebViewSupported",
      "(Landroid/content/Context;)Z", activity.object());
  if (supported) {
    AndroidUtils::instance()->startAuthentication(this, url);
    return;
  }
  DesktopAuthenticationListener* legacyAuth;
  legacyAuth = new DesktopAuthenticationListener(this);
  legacyAuth->start(task, codeChallenge, codeChallengeMethod, emailAddress);

  connect(legacyAuth, &AuthenticationListener::completed, this,
          &AndroidAuthenticationListener::completed);
  connect(legacyAuth, &AuthenticationListener::failed, this,
          &AndroidAuthenticationListener::failed);
}
