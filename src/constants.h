/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#include <QDateTime>
#include <QString>

namespace Constants {

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
bool inProduction();
const QString& getStagingServerAddress();
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

// The prefix for the user-agent requests
constexpr const char* NETWORK_USERAGENT_PREFIX = "MozillaVPN";

// The file name for the logging
constexpr const char* LOG_FILE_NAME = "mozillavpn.log";

// The localization filename prefix. The real file name should be called:
// `LOCALIZER_FILENAME_PREFIX` + '_' + languageCode + ".qm". For instance:
// `mozillavpn_it.qm
constexpr const char* LOCALIZER_FILENAME_PREFIX = "mozillavpn";

// Deep-Link Scheme for MacOS Authentication
constexpr const char* DEEP_LINK_SCHEME = "mozilla-vpn";

// The app product name in guardian
constexpr const char* AUTH_PROD_NAME = "vpn";

// The IAP plan on FxA
constexpr const char* IAP_PLANS = "guardian-vpn";

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

constexpr const char* ADDON_PRODUCTION_KEY =
    ":/addons_signature/production.der";
constexpr const char* ADDON_STAGING_KEY = ":/addons_signature/staging.der";

// A settings group that will contain all addons related settings.
constexpr const char* ADDONS_SETTINGS_GROUP = "addons";

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

namespace Timers {

#if defined(UNIT_TEST)
#  define TIMEREXPR(functionName, releaseValue, debugValue, testingValue) \
    inline std::chrono::milliseconds functionName() { return testingValue; }
#else
#  define TIMEREXPR(functionName, releaseValue, debugValue, testingValue) \
    inline std::chrono::milliseconds functionName() noexcept {            \
      return Constants::inProduction() ? releaseValue : debugValue;       \
    }
#endif
using namespace std::chrono_literals;
// How often we check for updates
TIMEREXPR(releaseMonitor, 6h, 4s, 0s)
// How often should we do periodic things (i.e update Serverlist)
TIMEREXPR(schedulePeriodicTask, 60min, 5min, 0ms)
// How often we should check for a Captive portal
TIMEREXPR(captivePortalRequest, 10s, 4s, 0ms)
// How fast the animated icon should move
TIMEREXPR(statusIconAnimation, 200ms, 200ms, 0ms)
// How often glean pings are sent
TIMEREXPR(gleanTimeout, 20min, 20min, 0ms)
#undef TIMEREXPR
}  // namespace Timers

constexpr const char* API_PRODUCTION_URL = "https://vpn.mozilla.org";
constexpr const char* API_STAGING_URL =
    "https://stage.guardian.nonprod.cloudops.mozgcp.net";

constexpr auto CRASH_PRODUCTION_URL =
    "https://crash-reports.mozilla.com/submit";
constexpr auto CRASH_STAGING_URL = "https://crash-reports.allizom.org/submit";

constexpr const char* LOGO_URL = ":/ui/resources/logo-dock.png";

constexpr const char* APPLE_SUBSCRIPTIONS_URL =
    "https://apps.apple.com/account/subscriptions";

constexpr const char* GOOGLE_SUBSCRIPTIONS_URL =
    "https://play.google.com/store/account/subscriptions";

#define PRODBETAEXPR(type, functionName, prod, beta) \
  inline type functionName() { return Constants::inProduction() ? prod : beta; }

constexpr const char* MOZILLA_VPN_SUMO_URL =
    "https://support.mozilla.org/products/firefox-private-network-vpn";

constexpr const char* SUMO_DNS =
    "https://support.mozilla.org/kb/how-do-i-change-my-dns-settings";

constexpr const char* SUMO_PRIVACY =
    "https://support.mozilla.org/kb/how-do-i-change-my-privacy-features";

constexpr const char* SUMO_EXCLUDED_APPS =
    "https://support.mozilla.org/kb/split-tunneling-app-permissions";

constexpr const char* SUMO_DEVICES =
    "https://support.mozilla.org/kb/"
    "how-add-devices-your-mozilla-vpn-subscription";

constexpr const char* SUMO_MULTIHOP =
    "https://support.mozilla.org/kb/"
    "multi-hop-encrypt-your-data-twice-enhanced-security";

PRODBETAEXPR(QString, contactSupportUrl, "https://accounts.firefox.com/support",
             "https://accounts.stage.mozaws.net/support")

PRODBETAEXPR(QString, addonBaseUrl,
             "https://archive.mozilla.org/pub/vpn/addons/releases/latest/",
             Constants::envOrDefault(
                 "MZ_ADDON_URL",
                 "https://mozilla-mobile.github.io/mozilla-vpn-client/addons/"))

PRODBETAEXPR(QString, captivePortalUrl, "http://%1/success.txt",
             Constants::envOrDefault("MZ_CAPTIVE_PORTAL_URL",
                                     "http://%1/success.txt"));

constexpr const char* BALROG_PROD_HOSTNAME = "aus5.mozilla.org";
constexpr const char* AUTOGRAPH_PROD_FINGERPRINTS[] = {
    // root-ca-production-amo 2015-03-17
    "97e8ba9cf12fb3de53cc42a4e6577ed64df493c247b414fea036818d3823560e",
    // root-ca-production-amo 2024-02-01
    // https://bugzilla.mozilla.org/show_bug.cgi?id=1882147
    "c8a80e9afaef4e219b6fb5d7a71d0f101223bac5001ac28f9b0d43dc59a106db",
    nullptr  // list termination
};

constexpr const char* BALROG_STAGE_HOSTNAME =
    "stage.balrog.nonprod.cloudops.mozgcp.net";
constexpr const char* AUTOGRAPH_STAGE_FINGERPRINTS[] = {
    // cas-new-2024-03-12
    "c0f05d59b1fde25780854c32fae8faba8481c233b4c1d390cca5f2cea81930ee",
    // cas-cur-2024-03-12
    "45c37f3a09a6d70e0fa321fb29753ba7998f1259b32772768f23ccdc24836798",
    // test.addons.signing.root.ca-2021-02-11
    "3c01446abe9036cea9a09acaa3a520ac628f20a7ae32ce861cb2efb70fa0c745",
    nullptr  // list termination
};

PRODBETAEXPR(qint64, keyRegeneratorTimeSec, 604800, 300);

PRODBETAEXPR(QString, upgradeToAnnualUrl,
             "https://www.mozilla.org/products/vpn/"
             "?utm_medium=mozillavpn&utm_source=account#pricing",
             "https://www-dev.allizom.org/products/vpn/"
             "?utm_medium=mozillavpn&utm_source=account#pricing")

#undef PRODBETAEXPR

#if defined(MZ_ADJUST)
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

#if defined(_WIN32)  // Avoid using MZ_WINDOWS here as it conflicts with
                     // MZ_DUMMY on Windows
// Credential key for windows
constexpr const wchar_t* WINDOWS_CRED_KEY = L"Mozilla VPN";
#endif

#ifdef MZ_ANDROID
constexpr const char* ANDROID_LOG_NAME = "mozillavpn";

constexpr const char* GOOGLE_PLAYSTORE_URL =
    "https://play.google.com/store/apps/details?id=org.mozilla.firefox.vpn";
#endif

#ifdef MZ_IOS
constexpr const char* APPLE_STORE_URL =
    "https://apps.apple.com/us/app/mozilla-vpn-secure-private/id1489407738";
#endif

// TODO: #if defined(MZ_LINUX) - but it breaks dummyvpn
constexpr const char* LINUX_CRYPTO_SETTINGS_KEY =
    "org.mozilla.vpn.cryptosettings";
constexpr const char* LINUX_CRYPTO_SETTINGS_DESC =
    "VPN settings encryption key";
// TODO: #endif

constexpr const char* LINUX_APP_ID = "org.mozilla.vpn";

};  // namespace Constants

#endif  // CONSTANTS_H
