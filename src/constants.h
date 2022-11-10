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
const QString& getStagingServerAddress();
void setStaging();

// Project version and build strings.
QString versionString();
QString buildNumber();
QString envOrDefault(const QString& name, const QString& defaultValue);

// Number of msecs for the captive-portal block alert.
constexpr uint32_t CAPTIVE_PORTAL_ALERT_MSEC = 4000;

// Number of msecs for the unsecured network alert.
constexpr uint32_t UNSECURED_NETWORK_ALERT_MSEC = 4000;

// Number of msecs for the server unavailable alert.
constexpr uint32_t SERVER_UNAVAILABLE_ALERT_MSEC = 4000;

// Number of msecs for the new in app message alert.
constexpr uint32_t NEW_IN_APP_MESSAGE_ALERT_MSEC = 4000;

// Number of recent connections to retain.
constexpr int RECENT_CONNECTIONS_MAX_COUNT = 5;

// Cooldown period for unresponsive servers
constexpr uint32_t SERVER_UNRESPONSIVE_COOLDOWN_SEC = 300;

// Number of msecs for max runtime of the connection benchmarks.
constexpr uint32_t BENCHMARK_MAX_BITS_UPLOAD = 80000000;  // 10 Megabyte
constexpr uint32_t BENCHMARK_MAX_DURATION_PING = 3000;
constexpr uint32_t BENCHMARK_MAX_DURATION_TRANSFER = 15000;
constexpr uint32_t BENCHMARK_THRESHOLD_SPEED_FAST = 25000000;    // 25 Megabit
constexpr uint32_t BENCHMARK_THRESHOLD_SPEED_MEDIUM = 10000000;  // 10 Megabit
constexpr const char* BENCHMARK_DOWNLOAD_URL =
    "https://archive.mozilla.org/pub/vpn/speedtest/50m.data";
// TODO: Add url for upload benchmark
constexpr const char* BENCHMARK_UPLOAD_URL = "";

#if defined(UNIT_TEST)
#  define CONSTEXPR(type, functionName, releaseValue, debugValue, \
                    testingValue)                                 \
    inline type functionName() { return testingValue; }
#else
#  define CONSTEXPR(type, functionName, releaseValue, debugValue, \
                    testingValue)                                 \
    inline type functionName() {                                  \
      return inProduction() ? releaseValue : debugValue;          \
    }
#endif

// Any 6 hours, a new check
CONSTEXPR(uint32_t, releaseMonitorMsec, 21600000, 4000, 0)

// in milliseconds, how often we should fetch the server list, the account and
// so on.
CONSTEXPR(uint32_t, schedulePeriodicTaskTimerMsec, 3600000, 30000, 0)

// how often we check the captive portal when the VPN is on.
CONSTEXPR(uint32_t, captivePortalRequestTimeoutMsec, 10000, 4000, 0)

// How fast the animated icon should move
CONSTEXPR(uint32_t, statusIconAnimationMsec, 200, 200, 0)

// How often glean pings are sent
CONSTEXPR(uint32_t, gleanTimeoutMsec, 1200000, 4000, 0)

// How often to check in on the controller state
CONSTEXPR(uint32_t, controllerPeriodicStateRecorderMsec, 10800000, 60000, 0)

#undef CONSTEXPR

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return inProduction() ? prod : beta; }

constexpr const char* API_PRODUCTION_URL = "https://vpn.mozilla.org";
constexpr const char* API_STAGING_URL =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";

constexpr const char* ADDON_PRODUCTION_URL =
    "https://archive.mozilla.org/pub/vpn/addons/releases/latest/";
constexpr const char* ADDON_PRODUCTION_KEY =
    ":/addons_signature/production.der";

constexpr const char* ADDON_STAGING_URL =
    "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/";
constexpr const char* ADDON_STAGING_KEY = ":/addons_signature/staging.der";

constexpr auto CRASH_PRODUCTION_URL =
    "https://crash-reports.mozilla.com/submit";
constexpr auto CRASH_STAGING_URL = "https://crash-reports.allizom.org/submit";

constexpr const char* LOGO_URL = ":/nebula/resources/logo-dock.png";

constexpr const char* APPLE_SUBSCRIPTIONS_URL =
    "https://apps.apple.com/account/subscriptions";

constexpr const char* GOOGLE_SUBSCRIPTIONS_URL =
    "https://play.google.com/store/account/subscriptions";

constexpr const char* ADDON_SETTINGS_GROUP = "addons";

PRODBETAEXPR(QString, fxaApiBaseUrl, "https://api.accounts.firefox.com",
             envOrDefault("MVPN_FXA_API_BASE_URL",
                          "https://api-accounts.stage.mozaws.net"))
PRODBETAEXPR(const char*, fxaUrl, "https://accounts.firefox.com",
             "https://accounts.stage.mozaws.net")
PRODBETAEXPR(
    const char*, balrogUrl,
    "https://aus5.mozilla.org/json/1/FirefoxVPN/%1/%2/release/update.json",
    "https://aus5.mozilla.org/json/1/FirefoxVPN/%1/%2/release-cdntest/"
    "update.json");
constexpr const char* AUTOGRAPH_ROOT_CERT_FINGERPRINT =
    "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e";

PRODBETAEXPR(const char*, relayUrl, "https://relay.firefox.com",
             "https://stage.fxprivaterelay.nonprod.cloudops.mozgcp.net");

PRODBETAEXPR(QString, privacyBundleProductId, "prod_MIex7Q079igFZJ",
             "prod_LcfR3EzYMVJlZQ");

PRODBETAEXPR(qint64, keyRegeneratorTimeSec, 604800, 300);

#undef PRODBETAEXPR

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

constexpr const char* PLACEHOLDER_USER_DNS = "127.0.0.1";

#if defined(MVPN_ADJUST)
// These are the two auto-generated token from the Adjust dashboard for the
// "Subscription Completed" event. We have two since in the Adjust dashboard we
// have defined two apps for iOS and Android with a event token each.
constexpr const char* ADJUST_SUBSCRIPTION_COMPLETED =
#  if defined(MVPN_IOS)
    "jl72xm"
#  elif defined(MVPN_ANDROID)
    "o1mn9m"
#  else
    ""
#  endif
    ;
#endif

};  // namespace Constants

#endif  // CONSTANTS_H
