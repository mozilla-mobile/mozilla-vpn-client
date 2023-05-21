/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#include <QString>

namespace Constants {

enum ApiEndpoint {
  Adjust,
  FeatureList,
};

/**
 * @brief expose the API URL with the requested path
 */
QString apiUrl(ApiEndpoint endpoint);

// Returns true if we are in a production environment.
bool inProduction();
void setStaging();

/**
 * @brief In staging only, override the version string for testing
 * purposes.
 */
void setVersionOverride(const QString& versionOverride);

// Project version and build strings.
QString versionString();
QString buildNumber();
QString envOrDefault(const QString& name, const QString& defaultValue);

constexpr const char* ADDON_PRODUCTION_KEY =
    ":/addons_signature/production.der";
constexpr const char* ADDON_STAGING_KEY = ":/addons_signature/staging.der";

constexpr const char* ADDON_SETTINGS_GROUP = "addons";

constexpr const char* PLATFORM_NAME =
#if defined(MZ_IOS)
    "ios"
#elif defined(MZ_MACOS)
    "macos"
#elif defined(MZ_LINUX)
    "linux"
#elif defined(MZ_ANDROID)
    "android"
#elif defined(MZ_WINDOWS)
    "windows"
#elif defined(UNIT_TEST) || defined(MZ_DUMMY)
    "dummy"
#else
#  error "Unsupported platform"
#endif
    ;

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

PRODBETAEXPR(const char*, fxaUrl, "https://accounts.firefox.com",
             "https://accounts.stage.mozaws.net")

PRODBETAEXPR(QString, fxaApiBaseUrl, "https://api.accounts.firefox.com",
             Constants::envOrDefault("MZ_FXA_API_BASE_URL",
                                     "https://api-accounts.stage.mozaws.net"))

#undef PRODBETAEXPR

#ifdef SENTRY_ENABLED
constexpr const char* SENTRY_DSN_ENDPOINT = SENTRY_DSN;
constexpr const char* SENTRY_ENVELOPE_INGESTION = SENTRY_ENVELOPE_ENDPOINT;
#else
constexpr const char* SENTRY_DSN_ENDPOINT = "";
constexpr const char* SENTRY_ENVELOPE_INGESTION = "";
#endif

};  // namespace Constants

#endif  // CONSTANTS_H
