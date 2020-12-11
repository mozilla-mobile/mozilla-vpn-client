/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidauthenticationlistener.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "platforms/android/androidutils.h"

#include <QAndroidJniObject>
#include <QtAndroid>
#include <jni.h>
#include "tasks/authenticate/desktopauthenticationlistener.h"

namespace {
Logger logger(LOG_ANDROID, "AndroidAuthenticationListener");
}

AndroidAuthenticationListener::AndroidAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MVPN_COUNT_CTOR(AndroidAuthenticationListener);
  logger.log() << "Android authentication litener";
}

AndroidAuthenticationListener::~AndroidAuthenticationListener() {
  MVPN_COUNT_DTOR(AndroidAuthenticationListener);
}

void AndroidAuthenticationListener::start(MozillaVPN* vpn, QUrl& url,
                                          QUrlQuery& query) {
  Q_UNUSED(vpn);

  logger.log() << "Authenticationlistener initialize";

  url.setQuery(query);

  QAndroidJniObject activity = QtAndroid::androidActivity();
  jboolean supported = QAndroidJniObject::callStaticMethod<jboolean>(
      "com/mozilla/vpn/PackageManagerHelper", "isWebViewSupported",
      "(Landroid/content/Context;)Z", activity.object());
  if (supported) {
    AndroidUtils::instance()->startAuthentication(this, url);
    return;
  }
  m_legacyAuth = new DesktopAuthenticationListener(this);
  m_legacyAuth->start(vpn, url, query);

  connect(m_legacyAuth, &AuthenticationListener::completed,
          [this](const QString& code) { emit this->completed(code); });
  connect(m_legacyAuth, &AuthenticationListener::failed,
          [this](const ErrorHandler::ErrorType error) {
            emit this->failed(error);
          });
}
