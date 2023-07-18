/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// 1. Define the SETTING macro
// 2. include this file
// 3. undefine the SETTING macro

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#  error No supported
#endif

#define SETTING_BOOL(getter, ...) SETTING(bool, toBool, getter, __VA_ARGS__)

#define SETTING_BYTEARRAY(getter, ...) \
  SETTING(QByteArray, toByteArray, getter, __VA_ARGS__)

#define SETTING_DATETIME(getter, ...) \
  SETTING(QDateTime, toDateTime, getter, __VA_ARGS__)

#define SETTING_INT(getter, ...) SETTING(int, toInt, getter, __VA_ARGS__)

#define SETTING_INT64(getter, ...) \
  SETTING(qint64, toLongLong, getter, __VA_ARGS__)

#define SETTING_STRING(getter, ...) \
  SETTING(QString, toString, getter, __VA_ARGS__)

#define SETTING_STRINGLIST(getter, ...) \
  SETTING(QStringList, toStringList, getter, __VA_ARGS__)

SETTING_BOOL(addonCustomServer,        // getter
             setAddonCustomServer,     // setter
             removeAddonCustomServer,  // remover
             hasAddonCustomServer,     // has
             "addon/customServer",     // key
             false,                    // default value
             false,                    // user setting
             false,                    // remove when reset
             false                     // sensitive (do not log)
)

SETTING_STRING(addonCustomServerAddress,        // getter
               setAddonCustomServerAddress,     // setter
               removeAddonCustomServerAddress,  // remover
               hasAddonCustomServerAddress,     // has
               "addon/customServerAddress",     // key
               AppConstants::addonBaseUrl(),    // default value
               false,                           // user setting
               false,                           // remove when reset
               false                            // sensitive (do not log)
)

SETTING_BOOL(addonProdKeyInStaging,        // getter
             setAddonProdKeyInStaging,     // setter
             removeAddonProdKeyInStaging,  // remover
             hasAddonProdKeyInStaging,     // has
             "addon/prodKeyInStaging",     // key
             false,                        // default value
             false,                        // user setting
             false,                        // remove when reset
             true                          // sensitive (do not log)
)

#if defined(MZ_ADJUST)
SETTING_BOOL(adjustActivatable,        // getter
             setAdjustActivatable,     // setter
             removeAdjustActivatable,  // remover
             hasAdjustActivatable,     // has
             "adjustActivatable",      // key
             false,                    // default value
             false,                    // user setting
             false,                    // remove when reset
             false                     // sensitive (do not log)
)
#endif

SETTING_STRINGLIST(featuresFlippedOff,        // getter
                   setFeaturesFlippedOff,     // setter
                   removeFeaturesFlippedOff,  // remover
                   hasFeaturesFlippedOff,     // has
                   "featuresFlippedOff",      // key
                   QStringList(),             // default value
                   false,                     // user setting
                   false,                     // remove when reset
                   false                      // sensitive (do not log)
)

SETTING_STRINGLIST(featuresFlippedOn,        // getter
                   setFeaturesFlippedOn,     // setter
                   removeFeaturesFlippedOn,  // remover
                   hasFeaturesFlippedOn,     // has
                   "featuresFlippedOn",      // key
                   QStringList(),            // default value
                   false,                    // user setting
                   false,                    // remove when reset
                   false                     // sensitive (do not log)
)

// TODO - This would be better named "telemetryEnabled", but as we already
// shipped with it called gleanEnabled it's non-trivial to change
// the name. https://github.com/mozilla-mobile/mozilla-vpn-client/issues/2050
SETTING_BOOL(gleanEnabled,        // getter
             setGleanEnabled,     // setter
             removeGleanEnabled,  // remover
             hasGleanEnabled,     // has
             "gleanEnabled",      // key
             true,                // default value
             true,                // user setting
             false,               // remove when reset
             false                // sensitive (do not log)
)

SETTING_DATETIME(installationTime,        // getter
                 setInstallationTime,     // setter
                 removeInstallationTime,  // remover
                 hasInstallationTime,     // has
                 "installationTime",      // key
                 QDateTime(),             // default value
                 false,                   // user setting
                 false,                   // remove when reset
                 false                    // sensitive (do not log)
)

SETTING_STRING(installedVersion,        // getter
               setInstalledVersion,     // setter
               removeInstalledVersion,  // remover
               hasInstalledVersion,     // has
               "installedVersion",      // key
               "",                      // default value
               false,                   // user setting
               false,                   // remove when reset
               false                    // sensitive (do not log)
)

SETTING_STRING(languageCode,        // getter
               setLanguageCode,     // setter
               removeLanguageCode,  // remover
               hasLanguageCode,     // has
               "languageCode",      // key
               "",                  // default value
               true,                // user setting
               false,               // remove when reset
               false                // sensitive (do not log)
)

SETTING_STRING(previousLanguageCode,        // getter
               setPreviousLanguageCode,     // setter
               removePreviousLanguageCode,  // remover
               hasPreviousLanguageCode,     // has
               "previousLanguageCode",      // key
               "",                          // default value
               true,                        // user setting
               false,                       // remove when reset
               false                        // sensitive (do not log)
)

SETTING_BOOL(systemLanguageCodeMigrated,        // getter
             setSystemLanguageCodeMigrated,     // setter
             removeSystemLanguageCodeMigrated,  // remover
             hasSystemLanguageCodeMigrated,     // has
             "systemLanguageCodeMigrated",      // key
             false,                             // default value
             false,                             // user setting
             true,                              // remove when reset
             false                              // sensitive (do not log)
)

SETTING_DATETIME(updateTime,        // getter
                 setUpdateTime,     // setter
                 removeUpdateTime,  // remover
                 hasUpdateTime,     // has
                 "updateTime",      // key
                 QDateTime(),       // default value
                 false,             // user setting
                 false,             // remove when reset
                 false              // sensitive (do not log)
)

SETTING_STRING(userEmail,        // getter
               setUserEmail,     // setter
               removeUserEmail,  // remover
               hasUserEmail,     // has
               "user/email",     // key
               "",               // default value
               false,            // user setting
               true,             // remove when reset
               true              // sensitive (do not log)
)

#define DEFAULT_THEME "main"

SETTING_STRING(theme,          // getter
               setTheme,       // setter
               removeTheme,    // remover
               hasTheme,       // has
               "theme",        // key
               DEFAULT_THEME,  // default value
               true,           // user setting
               true,           // remove when reset
               false           // sensitive (do not log)
)

// This setting is only intended for running the functional tests.
SETTING_BOOL(localhostRequestsOnly,        // Feature ID
             setLocalhostRequestsOnly,     // setter
             removeLocalhostRequestsOnly,  // remover
             hasLocalhostRequestsOnly,     // has
             "localhostRequestOnly",       // key
             false,                        // default value
             false,                        // user setting
             true,                         // remove when reset
             false                         // sensitive (do not log)
)

SETTING_STRING(token,        // getter
               setToken,     // setter
               removeToken,  // remover
               hasToken,     // has
               "token",      // key
               "",           // default value
               false,        // user setting
               true,         // remove when reset
               true          // sensitive (do not log)
)

SETTING_STRING(sentryDSN,        // getter
               setSentryDSN,     // setter
               removeSentryDSN,  // remover
               hasSentryDSN,     // has
               "sentry/dsn",     // key
               "",               // default value
               false,            // user setting
               true,             // remove when reset
               true              // sensitive (do not log)
)

SETTING_STRING(sentryEndpoint,        // getter
               setSentryEndpoint,     // setter
               removeSentryEndpoint,  // remover
               hasSentryEndpoint,     // has
               "sentry/endpoint",     // key
               "",                    // default value
               false,                 // user setting
               true,                  // remove when reset
               true                   // sensitive (do not log)
)

// The app must implement its settings list file.
#include "appsettingslist.h"
