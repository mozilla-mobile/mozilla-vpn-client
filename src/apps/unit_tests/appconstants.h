/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include "constants.h"

namespace AppConstants {

// This is used by SettingsHolder to configure the QSetting file.
constexpr const char* SETTINGS_APP_NAME = "UnitTest";

// The localization filename prefix. The real file name should be called:
// `LOCALIZER_FILENAME_PREFIX` + '_' + languageCode + ".qm". For instance:
// `mozillavpn_it.qm
constexpr const char* LOCALIZER_FILENAME_PREFIX = "unit_test";

// The prefix for the user-agent requests
constexpr const char* NETWORK_USERAGENT_PREFIX = "UnitTest";

// The file name for the logging
constexpr const char* LOG_FILE_NAME = "unit-test.txt";

#if defined(__APPLE__)
// This is the name of the service to encrypt the settings file
constexpr const char* CRYPTO_SETTINGS_SERVICE = "App UnitTest";

// Fallback. When an unsigned/un-notarized app is executed in
// command-line mode, it could fail the fetching of its own bundle id.
constexpr const char* MACOS_FALLBACK_APP_ID = "org.mozilla.macos.AppUnitTest";
constexpr const char* IOS_FALLBACK_APP_ID = "org.mozilla.ios.AppUnitTest";
#endif

//#if defined(MZ_WINDOWS)
// Credential key for windows
constexpr const wchar_t* WINDOWS_CRED_KEY = L"Unit Test";
//#endif

#if defined(MZ_ANDROID)
constexpr const char* ANDROID_LOG_NAME = "unittest";
#endif

constexpr const char* LINUX_CRYPTO_SETTINGS_KEY =
    "org.mozilla.unittest.cryptosettings";
constexpr const char* LINUX_CRYPTO_SETTINGS_DESC =
    "UnitTest settings encryption key";

// Communication pipe between instances
#if defined(MZ_WINDOWS)
constexpr const char* UI_PIPE = "\\\\.\\pipe\\unittest.ui";
#else
constexpr const char* UI_PIPE = "/tmp/unittest.ui.sock";
#endif

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

PRODBETAEXPR(QString, addonBaseUrl,
             "https://archive.mozilla.org/pub/unittest/addons/releases/latest/",
             Constants::envOrDefault(
                 "MZ_ADDON_URL",
                 "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/"))

#undef PRODBETAEXPR

};  // namespace AppConstants

#endif  // APPCONSTANTS_H
