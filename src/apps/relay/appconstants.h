/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include "constants.h"

namespace AppConstants {

constexpr const char* API_PRODUCTION_URL = "https://vpn.mozilla.org";
constexpr const char* API_STAGING_URL =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";

/**
 * @brief expose the API base URL for guardian
 */
QString apiBaseUrl();

/**
 * @brief retrieve the staging server address
 */
const QString& getStagingServerAddress();
void setStaging();

// This is used by SettingsHolder to configure the QSetting file.
constexpr const char* SETTINGS_APP_NAME = "relay";

// The prefix for the user-agent requests
constexpr const char* NETWORK_USERAGENT_PREFIX = "RelayApp";

// The file name for the logging
constexpr const char* LOG_FILE_NAME = "relayapp.txt";

// The localization filename prefix. The real file name should be called:
// `LOCALIZER_FILENAME_PREFIX` + '_' + languageCode + ".qm". For instance:
// `mozillavpn_it.qm
constexpr const char* LOCALIZER_FILENAME_PREFIX = "relay";

// The app product name in guardian
constexpr const char* AUTH_PROD_NAME = "vpn";  // TODO!

#if defined(__APPLE__)
// This is the name of the service to encrypt the settings file
constexpr const char* CRYPTO_SETTINGS_SERVICE = "Relay App";

// Fallback. When an unsigned/un-notarized app is executed in
// command-line mode, it could fail the fetching of its own bundle id.
constexpr const char* MACOS_FALLBACK_APP_ID = "org.mozilla.macos.Relay";
constexpr const char* IOS_FALLBACK_APP_ID = "org.mozilla.ios.Relay";
#endif

#if defined(MZ_WINDOWS)
// Credential key for windows
constexpr const wchar_t* WINDOWS_CRED_KEY = L"Relay App";
#endif

#if defined(MZ_ANDROID)
constexpr const char* ANDROID_LOG_NAME = "relay";

constexpr const char* GOOGLE_PLAYSTORE_URL =
    "https://play.google.com/store/apps/details?id=TODO";  // TODO
#endif

#ifdef MZ_IOS
constexpr const char* APPLE_STORE_URL =
    "https://apps.apple.com/us/app/mozilla-vpn-secure-private/id123456789";  // TODO
constexpr const char* APPLE_STORE_REVIEW_URL =
    "https://apps.apple.com/app/id123456789?action=write-review";  // TODO
#endif

#if defined(MZ_LINUX)
constexpr const char* LINUX_CRYPTO_SETTINGS_KEY =
    "org.mozilla.relay.cryptosettings";
constexpr const char* LINUX_CRYPTO_SETTINGS_DESC =
    "Relay settings encryption key";
#endif

// Communication pipe between instances
#if defined(MZ_WINDOWS)
constexpr const char* UI_PIPE = "\\\\.\\pipe\\relayapp.ui";
#else
constexpr const char* UI_PIPE = "/tmp/relayapp.ui.sock";
#endif

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

PRODBETAEXPR(
    QString, addonBaseUrl,
    "https://archive.mozilla.org/pub/relayapp/addons/releases/latest/",
    Constants::envOrDefault(
        "MZ_ADDON_URL",
        "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/relay/"))

#undef PRODBETAEXPR

constexpr const char* LOGO_URL = ":/ui/resources/logo-dock.png";

};  // namespace AppConstants

#endif  // APPCONSTANTS_H
