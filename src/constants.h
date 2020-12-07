/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdint.h>

namespace Constants {

// This value is the fallback in case apple is slower than the user. This
// should never happen.
constexpr const char* SUBSCRIPTION_CURRENCY_VALUE_USD = "4.99$";

#if defined(UNIT_TEST)
#  define CONSTEXPR(type, what, releaseValue, debugValue, testingValue) \
    constexpr type what = testingValue;
#elif defined(QT_DEBUG)
#  define CONSTEXPR(type, what, releaseValue, debugValue, tstingValue) \
    constexpr type what = debugValue;
#else
#  define CONSTEXPR(type, what, releaseValue, debugValue, tstingValue) \
    constexpr type what = releaseValue;
#endif

// Let's refresh the IP address any 10 minutes (in milliseconds).
CONSTEXPR(uint32_t, IPADDRESS_TIMER_MSEC, 600000, 10000, 0)

// Let's check the connection status any second.
CONSTEXPR(uint32_t, CHECKSTATUS_TIMER_MSEC, 1000, 1000, 0)

// Number of points for the charts.
CONSTEXPR(int, CHARTS_MAX_POINTS, 30, 30, 30);

// Any 6 hours, a new check
CONSTEXPR(uint32_t, RELEASE_MONITOR_MSEC, 21600000, 4000, 0)

// in milliseconds, how often we should fetch the server list and the account.
CONSTEXPR(uint32_t, SCHEDULE_ACCOUNT_AND_SERVERS_TIMER_MSEC, 3600000, 4000, 0)

// how often we check the captive portal when the VPN is on.
CONSTEXPR(uint32_t, CAPTIVEPORTAL_REQUEST_TIMEOUT_MSEC, 10000, 4000, 0)

// How fast the animated icon should move
CONSTEXPR(uint32_t, STATUSICON_ANIMATION_MSEC, 200, 200, 0)

#undef CONSTEXPR

#ifdef MVPN_PRODUCTION_MODE
#  define PRODBETAEXPR(type, what, prod, beta) constexpr type what = prod;
#else
#  define PRODBETAEXPR(type, what, prod, beta) constexpr type what = beta;
#endif

PRODBETAEXPR(const char*, API_URL, "https://vpn.mozilla.org",
             "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net")
PRODBETAEXPR(const char*, LOGO_URL, ":/ui/resources/logo-dock.png",
             ":/ui/resources/logo-dock-beta.png")

#undef PRODBETAEXPR

};  // namespace Constants
