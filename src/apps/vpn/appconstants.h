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
QString apiBaseUrl();

enum ApiEndpoint {
  Account,
  Adjust,
  CreateSupportTicket,
  CreateSupportTicketGuest,
  Device,
  DeviceWithPublicKeyArgument,
  DNSDetectPortal,
  FeatureList,
  Feedback,
  Heartbeat,
  IPInfo,
  LoginVerify,
  Products,
#ifdef MZ_ANDROID
  PurchasesAndroid,
#endif
#ifdef MZ_IOS
  PurchasesIOS,
#endif
#ifdef MZ_WASM
  PurchasesWasm,
#endif
  RedirectPrivacy,
  RedirectSubscriptionBlocked,
  RedirectTermsOfService,
  RedirectUpdateWithPlatformArgument,
  Servers,
  SubscriptionDetails,
  Versions,
};

/**
 * @brief expose the API URL with the requested path
 */
QString apiUrl(ApiEndpoint endpoint);

// Returns true if we are in a production environment.
const QString& getStagingServerAddress();
void setStaging();

// This is used by SettingsHolder to configure the QSetting file.
constexpr const char* SETTINGS_APP_NAME = "vpn";

// The prefix for the user-agent requests
constexpr const char* NETWORK_USERAGENT_PREFIX = "MozillaVPN";

// The file name for the logging
constexpr const char* LOG_FILE_NAME = "mozillavpn.txt";

// The localization filename prefix. The real file name should be called:
// `LOCALIZER_FILENAME_PREFIX` + '_' + languageCode + ".qm". For instance:
// `mozillavpn_it.qm
constexpr const char* LOCALIZER_FILENAME_PREFIX = "mozillavpn";

// Number of msecs for the captive-portal block alert.
constexpr uint32_t CAPTIVE_PORTAL_ALERT_MSEC = 4000;

// Number of msecs for the unsecured network alert.
constexpr uint32_t UNSECURED_NETWORK_ALERT_MSEC = 4000;

// Number of msecs for the server unavailable alert.
constexpr uint32_t SERVER_UNAVAILABLE_ALERT_MSEC = 4000;

// Number of msecs for the new in app message alert.
constexpr uint32_t NEW_IN_APP_MESSAGE_ALERT_MSEC = 4000;

// Default number of msecs for OS notifications.
constexpr uint32_t DEFAULT_OS_NOTIFICATION_MSEC = 4000;

// Number of recent connections to retain.
constexpr int RECENT_CONNECTIONS_MAX_COUNT = 2;

// Cooldown period for unresponsive servers
constexpr uint32_t SERVER_UNRESPONSIVE_COOLDOWN_SEC = 300;

// Number of msecs for max runtime of the connection benchmarks.
constexpr uint32_t BENCHMARK_MAX_BYTES_UPLOAD = 10485760;  // 10 Megabyte
constexpr uint32_t BENCHMARK_MAX_DURATION_PING = 3000;
constexpr uint32_t BENCHMARK_MAX_DURATION_TRANSFER = 15000;
constexpr uint32_t BENCHMARK_THRESHOLD_SPEED_FAST = 25000000;    // 25 Megabit
constexpr uint32_t BENCHMARK_THRESHOLD_SPEED_MEDIUM = 10000000;  // 10 Megabit

#if defined(UNIT_TEST)
#  define CONSTEXPR(type, functionName, releaseValue, debugValue, \
                    testingValue)                                 \
    inline type functionName() { return testingValue; }
#else
#  define CONSTEXPR(type, functionName, releaseValue, debugValue,   \
                    testingValue)                                   \
    inline type functionName() {                                    \
      return Constants::inProduction() ? releaseValue : debugValue; \
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

#ifdef SENTRY_ENABLED
constexpr const char* SENTRY_DSN_ENDPOINT = SENTRY_DSN;
constexpr const char* SENTRY_ENVELOPE_INGESTION = SENTRY_ENVELOPE_ENDPOINT;
#else
constexpr const char* SENTRY_DSN_ENDPOINT = "";
constexpr const char* SENTRY_ENVELOPE_INGESTION = "";
#endif

constexpr const char* API_PRODUCTION_URL = "https://vpn.mozilla.org";
constexpr const char* API_STAGING_URL =
    "https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net";

constexpr auto CRASH_PRODUCTION_URL =
    "https://crash-reports.mozilla.com/submit";
constexpr auto CRASH_STAGING_URL = "https://crash-reports.allizom.org/submit";

constexpr const char* LOGO_URL = ":/nebula/resources/logo-dock.png";

constexpr const char* APPLE_SUBSCRIPTIONS_URL =
    "https://apps.apple.com/account/subscriptions";

constexpr const char* GOOGLE_SUBSCRIPTIONS_URL =
    "https://play.google.com/store/account/subscriptions";

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

constexpr const char* MOZILLA_VPN_SUMO_URL =
    "https://support.mozilla.org/en-US/products/firefox-private-network-vpn";

PRODBETAEXPR(QString, addonBaseUrl,
             "https://archive.mozilla.org/pub/vpn/addons/releases/latest/",
             Constants::envOrDefault(
                 "MZ_ADDON_URL",
                 "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/"))

PRODBETAEXPR(QString, benchmarkDownloadUrl,
             "https://archive.mozilla.org/pub/vpn/speedtest/50m.data",
             Constants::envOrDefault(
                 "MZ_BENCHMARK_DOWNLOAD_URL",
                 "https://archive.mozilla.org/pub/vpn/speedtest/50m.data"));

PRODBETAEXPR(
    QString, benchmarkUploadUrl, "https://benchmark.vpn.mozilla.org/upload",
    Constants::envOrDefault(
        "MZ_BENCHMARK_UPLOAD_URL",
        "https://dev.vpn-network-benchmark.nonprod.webservices.mozgcp.net/"
        "upload"));

PRODBETAEXPR(QString, captivePortalUrl, "http://%1/success.txt",
             Constants::envOrDefault("MZ_CAPTIVE_PORTAL_URL",
                                     "http://%1/success.txt"));

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

#if defined(MVPN_ADJUST)
// These are the two auto-generated token from the Adjust dashboard for the
// "Subscription Completed" event. We have two since in the Adjust dashboard we
// have defined two apps for iOS and Android with a event token each.
constexpr const char* ADJUST_SUBSCRIPTION_COMPLETED =
#  if defined(MZ_IOS)
    "jl72xm"
#  elif defined(MZ_ANDROID)
    "o1mn9m"
#  else
    ""
#  endif
    ;
#endif

#if defined(__APPLE__)
// This is the name of the service to encrypt the settings file
constexpr const char* CRYPTO_SETTINGS_SERVICE = "Mozilla VPN";

// Fallback. When an unsigned/un-notarized app is executed in
// command-line mode, it could fail the fetching of its own bundle id.
constexpr const char* MACOS_FALLBACK_APP_ID = "org.mozilla.macos.FirefoxVPN";
constexpr const char* IOS_FALLBACK_APP_ID = "org.mozilla.ios.FirefoxVPN";
#endif

#if defined(MZ_WINDOWS)
// Credential key for windows
constexpr const wchar_t* WINDOWS_CRED_KEY = L"Mozilla VPN";
#endif

// Communication pipe between instances
#if defined(MZ_WINDOWS)
constexpr const char* UI_PIPE = "\\\\.\\pipe\\mozillavpn.ui";
#else
constexpr const char* UI_PIPE = "/tmp/mozillavpn.ui.sock";
#endif

#if defined(MZ_ANDROID)
constexpr const char* ANDROID_LOG_NAME = "mozillavpn";
#endif

// TODO: #if defined(MZ_LINUX) - but it breaks dummyvpn
constexpr const char* LINUX_CRYPTO_SETTINGS_KEY =
    "org.mozilla.vpn.cryptosettings";
constexpr const char* LINUX_CRYPTO_SETTINGS_DESC =
    "VPN settings encryption key";
// TODO: #endif

};  // namespace AppConstants

#endif  // APPCONSTANTS_H
