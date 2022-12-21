/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#include <QString>

namespace Constants {

// Returns true if we are in a production environment.
bool inProduction();
void setStaging();

// Project version and build strings.
QString versionString();
QString buildNumber();
QString envOrDefault(const QString& name, const QString& defaultValue);

constexpr const char* ADDON_PRODUCTION_KEY =
    ":/addons_signature/production.der";
constexpr const char* ADDON_STAGING_KEY = ":/addons_signature/staging.der";

constexpr const char* ADDON_SETTINGS_GROUP = "addons";

constexpr const char* PLATFORM_NAME =
#if defined(MVPN_IOS)
    "ios"
#elif defined(MVPN_MACOS)
    "macos"
#elif defined(MVPN_LINUX)
    "linux"
#elif defined(MVPN_ANDROID)
    "android"
#elif defined(MVPN_WINDOWS)
    "windows"
#elif defined(UNIT_TEST) || defined(MVPN_DUMMY)
    "dummy"
#else
#  error "Unsupported platform"
#endif
    ;

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

PRODBETAEXPR(
    QString, addonBaseUrl,
    "https://archive.mozilla.org/pub/vpn/addons/releases/latest/",
    envOrDefault("MVPN_ADDON_URL",
                 "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/"))

#undef PRODBETAEXPR

};  // namespace Constants

#endif  // CONSTANTS_H
