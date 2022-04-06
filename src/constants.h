/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

class QString;

namespace Constants {

// Returns true if we are in a production environment.
bool inProduction();
const QString& getStagingServerAddress();
void setStaging();

// Number of msecs for the captive-portal block alert.
constexpr uint32_t CAPTIVE_PORTAL_ALERT_MSEC = 4000;

// Number of msecs for the unsecured network alert.
constexpr uint32_t UNSECURED_NETWORK_ALERT_MSEC = 4000;

// Number of msecs for the server unavailable alert.
constexpr uint32_t SERVER_UNAVAILABLE_ALERT_MSEC = 4000;

// Number of recent connections to retain.
constexpr int RECENT_CONNECTIONS_MAX_COUNT = 5;

// Cooldown period for unresponsive servers
constexpr uint32_t SERVER_UNRESPONSIVE_COOLDOWN_SEC = 300;

// Number of msecs for max runtime of the connection benchmarks.
constexpr uint32_t BENCHMARK_MAX_DURATION_PING = 3000;
constexpr uint32_t BENCHMARK_MAX_DURATION_DOWNLOAD = 15000;
constexpr uint32_t BENCHMARK_THRESHOLD_SPEED_FAST = 25000000;    // 25 Megabit
constexpr uint32_t BENCHMARK_THRESHOLD_SPEED_MEDIUM = 10000000;  // 10 Megabit
constexpr const char* BENCHMARK_DOWNLOAD_URL =
    "https://archive.mozilla.org/pub/vpn/speedtest/50m.data";

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
CONSTEXPR(uint32_t, schedulePeriodicTaskTimerMsec, 3600000, 4000, 0)

// how often we check the captive portal when the VPN is on.
CONSTEXPR(uint32_t, captivePortalRequestTimeoutMsec, 10000, 4000, 0)

// How fast the animated icon should move
CONSTEXPR(uint32_t, statusIconAnimationMsec, 200, 200, 0)

// How often glean pings are sent
CONSTEXPR(uint32_t, gleanTimeoutMsec, 1200000, 4000, 0)

// How often we check the surveys to be executed (no network requests are done
// for this check)
CONSTEXPR(uint32_t, surveyTimerMsec, 300000, 4000, 0)

#undef CONSTEXPR

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return inProduction() ? prod : beta; }

constexpr const char* API_PRODUCTION_URL = "https://vpn.mozilla.org";
constexpr const char* API_STAGING_URL =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";

constexpr auto CRASH_PRODUCTION_URL =
    "https://crash-reports.mozilla.com/submit";
constexpr auto CRASH_STAGING_URL = "https://crash-reports.allizom.org/submit";

constexpr const char* LOGO_URL = ":/nebula/resources/logo-dock.png";

PRODBETAEXPR(const char*, fxaApiBaseUrl, "https://api.accounts.firefox.com",
             "https://api-accounts.stage.mozaws.net")
PRODBETAEXPR(const char*, fxaUrl, "https://accounts.firefox.com",
             "https://accounts.stage.mozaws.net")
PRODBETAEXPR(
    const char*, balrogUrl,
    "https://aus5.mozilla.org/json/1/FirefoxVPN/%1/%2/release/update.json",
    "https://stage.balrog.nonprod.cloudops.mozgcp.net/json/1/FirefoxVPN/%1/%2/"
    "release-cdntest/update.json");
PRODBETAEXPR(
    const char*, balrogRootCertFingerprint,
    "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e",
    "3c01446abe9036cea9a09acaa3a520ac628f20a7ae32ce861cb2efb70fa0c745");

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
