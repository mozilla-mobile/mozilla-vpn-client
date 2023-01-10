/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include "constants.h"

namespace AppConstants {

// This is used by SettingsHolder to configure the QSetting file.
constexpr const char* SETTINGS_APP_NAME = "FooBar";

// The prefix for the user-agent requests
constexpr const char* NETWORK_USERAGENT_PREFIX = "FooBar";

#if defined(__APPLE__)
// This is the name of the service to encrypt the settings file
constexpr const char* CRYPTO_SETTINGS_SERVICE = "App FooBar";

// Fallback. When an unsigned/un-notarized app is executed in
// command-line mode, it could fail the fetching of its own bundle id.
constexpr const char* MACOS_FALLBACK_APP_ID = "org.mozilla.macos.AppFooBar";
constexpr const char* IOS_FALLBACK_APP_ID = "org.mozilla.ios.AppFooBar";
#endif

#if defined(MZ_WINDOWS)
// Credential key for windows
constexpr const wchar_t* WINDOWS_CRED_KEY = L"Foo bar";
#endif

#if defined(MZ_ANDROID)
constexpr const char* ANDROID_LOG_NAME = "foobar";
#endif

#if defined(MZ_LINUX)
constexpr const char* LINUX_CRYPTO_SETTINGS_KEY =
    "org.mozilla.foobar.cryptosettings";
constexpr const char* LINUX_CRYPTO_SETTINGS_DESC =
    "FooBar settings encryption key";
#endif

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

PRODBETAEXPR(
    QString, addonBaseUrl,
    "https://archive.mozilla.org/pub/foobar/addons/releases/latest/",
    envOrDefault("MZ_ADDON_URL",
                 "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/"))

#undef PRODBETAEXPR

// TODO: add something here.

};  // namespace AppConstants

#endif  // APPCONSTANTS_H
