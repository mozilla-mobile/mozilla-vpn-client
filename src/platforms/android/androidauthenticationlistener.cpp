/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidauthenticationlistener.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "platforms/android/androidutils.h"

namespace {
Logger logger(LOG_ANDROID, "AndroidAuthenticationListener");
}

AndroidAuthenticationListener::AndroidAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MVPN_COUNT_CTOR(AndroidAuthenticationListener);

  logger.log() << "Android authentication listener";
}

AndroidAuthenticationListener::~AndroidAuthenticationListener() {
  MVPN_COUNT_DTOR(AndroidAuthenticationListener);
}

void AndroidAuthenticationListener::start(MozillaVPN* vpn, QUrl& url,
                                          QUrlQuery& query) {
  Q_UNUSED(vpn);

  logger.log() << "Authenticationlistener initialize";

  url.setQuery(query);

  AndroidUtils::instance()->startAuthentication(this, url);
}
