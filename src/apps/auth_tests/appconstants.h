/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include "constants.h"

namespace AppConstants {

/**
 * @brief expose the API base URL for guardian
 */
inline QString apiBaseUrl() {
  return "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";
}

// This is used by SettingsHolder to configure the QSetting file.
constexpr const char* SETTINGS_APP_NAME = "AuthTest";

// The localization filename prefix. The real file name should be called:
// `LOCALIZER_FILENAME_PREFIX` + '_' + languageCode + ".qm". For instance:
// `mozillavpn_it.qm
constexpr const char* LOCALIZER_FILENAME_PREFIX = "auth_test";

// Deep-Link Scheme for MacOS Authentication
constexpr const char* DEEP_LINK_SCHEME = "auth-test";

// The prefix for the user-agent requests. This needs to be kept as MozillaVPN
// to make guardian happy.
constexpr const char* NETWORK_USERAGENT_PREFIX = "MozillaVPN";

// The file name for the logging
constexpr const char* LOG_FILE_NAME = "auth-test.txt";

// The app product name in guardian
constexpr const char* AUTH_PROD_NAME = "vpn";

#if defined(__APPLE__)
// This is the name of the service to encrypt the settings file
constexpr const char* CRYPTO_SETTINGS_SERVICE = "App AuthTest";

// Fallback. When an unsigned/un-notarized app is executed in
// command-line mode, it could fail the fetching of its own bundle id.
constexpr const char* MACOS_FALLBACK_APP_ID = "org.mozilla.macos.AppAuthTest";
constexpr const char* IOS_FALLBACK_APP_ID = "org.mozilla.ios.AppAuthTest";
#endif

#if defined(MZ_WINDOWS)
// Credential key for windows
constexpr const wchar_t* WINDOWS_CRED_KEY = L"Auth Test";
#endif

#if defined(MZ_ANDROID)
constexpr const char* ANDROID_LOG_NAME = "authtest";
#endif

constexpr const char* LINUX_CRYPTO_SETTINGS_KEY =
    "org.mozilla.authtest.cryptosettings";
constexpr const char* LINUX_CRYPTO_SETTINGS_DESC =
    "AuthTest settings encryption key";

// Communication pipe between instances
#if defined(MZ_WINDOWS)
constexpr const char* UI_PIPE = "\\\\.\\pipe\\authtest.ui";
#else
constexpr const char* UI_PIPE = "/tmp/authtest.ui.sock";
#endif

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

PRODBETAEXPR(QString, addonBaseUrl,
             "https://archive.mozilla.org/pub/authtest/addons/releases/latest/",
             Constants::envOrDefault(
                 "MZ_ADDON_URL",
                 "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/"))

#undef PRODBETAEXPR

};  // namespace AppConstants

#endif  // APPCONSTANTS_H
