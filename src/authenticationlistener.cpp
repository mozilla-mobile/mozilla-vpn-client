/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationlistener.h"
#include "leakdetector.h"
#include "logger.h"

#ifdef MVPN_ANDROID
#include "platforms/android/androidauthenticationlistener.h"
#else
#include "tasks/authenticate/desktopauthenticationlistener.h"
#endif

namespace {
Logger logger(LOG_MAIN, "AuthenticationListener");
} // anonymous namespace

// static
AuthenticationListener *AuthenticationListener::create(QObject *parent)
{
#ifdef MVPN_ANDROID
    return new AndroidAuthenticationListener(parent);
#else
    return new DesktopAuthenticationListener(parent);
#endif
}

AuthenticationListener::AuthenticationListener(QObject *parent) : QObject(parent)
{
    MVPN_COUNT_CTOR(AuthenticationListener);
}

AuthenticationListener::~AuthenticationListener()
{
    MVPN_COUNT_DTOR(AuthenticationListener);
}
