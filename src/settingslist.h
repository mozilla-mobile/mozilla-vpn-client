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

#define SETTING_STRING(getter, ...) \
  SETTING(QString, toString, getter, __VA_ARGS__)

#define SETTING_STRINGLIST(getter, ...) \
  SETTING(QStringList, toStringList, getter, __VA_ARGS__)

// Please! Keep the alphabetic order!

SETTING_BOOL(captivePortalAlert,                               // getter
             setCaptivePortalAlert,                            // setter
             hasCaptivePortalAlert,                            // has
             "captivePortalAlert",                             // key
             FeatureCaptivePortal::instance()->isSupported(),  // default value
             false  // remove when reset
)

SETTING_STRINGLIST(captivePortalIpv4Addresses,     // getter
                   setCaptivePortalIpv4Addresses,  // setter
                   hasCaptivePortalIpv4Addresses,  // has
                   "captivePortal/ipv4Addresses",  // key
                   QStringList(),                  // default value
                   false                           // remove when reset
)

SETTING_STRINGLIST(captivePortalIpv6Addresses,     // getter
                   setCaptivePortalIpv6Addresses,  // setter
                   hasCaptivePortalIpv6Addresses,  // has
                   "captivePortal/ipv6Addresses",  // key
                   QStringList(),                  // default value
                   false                           // remove when reset
)

SETTING_BOOL(connectionChangeNotification,     // getter
             setConnectionChangeNotification,  // setter
             hasConnectionChangeNotification,  // has
             "connectionChangeNotification",   // key
             true,                             // default value
             false                             // remove when reset
)

SETTING_STRINGLIST(consumedSurveys,     // getter
                   setConsumedSurveys,  // setter
                   hasConsumedSurveys,  // has
                   "consumedSurveys",   // key
                   QStringList(),       // default value
                   false                // remove when reset
)

SETTING_STRING(currentServerCity,     // getter
               setCurrentServerCity,  // setter
               hasCurrentServerCity,  // has
               "currentServer/city",  // key
               "",                    // default value
               true                   // remove when reset
)

SETTING_STRING(currentServerCountry,     // getter
               setCurrentServerCountry,  // setter
               hasCurrentServerCountry,  // has
               "currentServer/country",  // key
               "",                       // default value
               true                      // remove when reset
)

SETTING_STRING(currentServerCountryCode,     // getter
               setCurrentServerCountryCode,  // setter
               hasCurrentServerCountryCode,  // has
               "currentServer/countryCode",  // key
               "",                           // default value
               true                          // remove when reset
)

SETTING_BOOL(developerUnlock,     // getter
             setDeveloperUnlock,  // setter
             hasDeveloperUnlock,  // has
             "developerUnlock",   // key
             false,               // default value
             false                // remove when reset
)

SETTING_STRING(deviceKeyVersion,     // getter
               setDeviceKeyVersion,  // setter
               hasDeviceKeyVersion,  // has
               "deviceKeyVersion",   // key
               "",                   // default value
               true                  // remove when reset
)

SETTING_BYTEARRAY(devices,     // getter
                  setDevices,  // setter
                  hasDevices,  // has
                  "devices",   // key
                  "",          // default value
                  true         // remove when reset
)

SETTING_STRINGLIST(devModeFeatureFlags,     // getter
                   setDevModeFeatureFlags,  // setter
                   hasDevModeFeatureFlags,  // has
                   "devmodeFeatureFlags",   // key
                   QStringList(),           // default value
                   false                    // remove when reset
)

SETTING_INT(dnsProvider,                           // getter
            setDNSProvider,                        // setter
            hasDNSProvider,                        // has
            "dnsProvider",                         // key
            SettingsHolder::DnsProvider::Gateway,  // default value
            false                                  // remove when reset
)

SETTING_STRING(entryServerCity,     // getter
               setEntryServerCity,  // setter
               hasEntryServerCity,  // has
               "entryServer/city",  // key
               nullptr,             // default value
               true                 // remove when reset
)

SETTING_STRING(entryServerCountryCode,     // getter
               setEntryServerCountryCode,  // setter
               hasEntryServerCountryCode,  // has
               "entryServer/countryCode",  // key
               nullptr,                    // default value
               true                        // remove when reset
)

SETTING_BOOL(featuresTourShown,     // getter
             setFeaturesTourShown,  // setter
             hasFeaturesTourShown,  // has
             "featuresTourShown",   // key
             false,                 // default value
             false                  // remove when reset
)

SETTING_BOOL(gleanEnabled,     // getter
             setGleanEnabled,  // setter
             hasGleanEnabled,  // has
             "gleanEnabled",   // key
             true,             // default value
             false             // remove when reset
)

SETTING_STRINGLIST(iapProducts,     // getter
                   setIapProducts,  // setter
                   hasIapProducts,  // has
                   "iapProducts",   // key
                   QStringList(),   // default value
                   true             // remove when reset
)

SETTING_DATETIME(installationTime,     // getter
                 setInstallationTime,  // setter
                 hasInstallationTime,  // has
                 "installationTime",   // key
                 QDateTime(),          // default value
                 false                 // remove when reset
)

SETTING_STRING(languageCode,     // getter
               setLanguageCode,  // setter
               hasLanguageCode,  // has
               "languageCode",   // key
               "",               // default value
               false             // remove when reset
)

SETTING_BOOL(localNetworkAccess,     // getter
             setLocalNetworkAccess,  // setter
             hasLocalNetworkAccess,  // has
             "localNetworkAccess",   // key
             false,                  // default value
             false                   // remove when reset
)

SETTING_STRINGLIST(missingApps,     // getter
                   setMissingApps,  // setter
                   hasMissingApps,  // has
                   "MissingApps",   // key
                   QStringList(),   // default value
                   false            // remove when reset
)

SETTING_BOOL(postAuthenticationShown,     // getter
             setPostAuthenticationShown,  // setter
             hasPostAuthenticationShown,  // has
             "postAuthenticationShown",   // key
             false,                       // default value
             true                         // remove when reset
)

SETTING_STRING(previousLanguageCode,     // getter
               setPreviousLanguageCode,  // setter
               hasPreviousLanguageCode,  // has
               "previousLanguageCode",   // key
               "",                       // default value
               false                     // remove when reset
)

SETTING_STRING(privateKey,     // getter
               setPrivateKey,  // setter
               hasPrivateKey,  // has
               "privateKey",   // key
               "",             // default value
               true            // remove when reset
)

SETTING_BOOL(protectSelectedApps,     // getter
             setProtectSelectedApps,  // setter
             hasProtectSelectedApps,  // has
             "protectSelectedApps",   // key
             false,                   // default value
             false                    // remove when reset
)

SETTING_STRING(publicKey,     // getter
               setPublicKey,  // setter
               hasPublicKey,  // has
               "publicKey",   // key
               "",            // default value
               true           // remove when reset
)

SETTING_STRINGLIST(recentConnections,     // getter
                   setRecentConnections,  // setter
                   hasRecentConnections,  // has
                   "recentConnections",   // key
                   QStringList(),         // default value
                   true                   // remove when reset
)

SETTING_STRINGLIST(seenFeatures,     // getter
                   setSeenFeatures,  // setter
                   hasSeenFeatures,  // has
                   "seenFeatures",   // key
                   QStringList(),    // default value
                   false             // remove when reset
)

SETTING_BYTEARRAY(servers,     // getter
                  setServers,  // setter
                  hasServers,  // has
                  "servers",   // key
                  "",          // default value
                  true         // remove when reset
)

SETTING_BYTEARRAY(serverExtras,     // getter
                  setServerExtras,  // setter
                  hasServerExtras,  // has
                  "serverExtras",   // key
                  "",               // default value
                  true              // remove when reset
)

SETTING_BOOL(serverSwitchNotification,     // getter
             setServerSwitchNotification,  // setter
             hasServerSwitchNotification,  // has
             "serverSwitchNotification",   // key
             true,                         // default value
             false                         // remove when reset
)

SETTING_STRING(stagingServerAddress,     // getter
               setStagingServerAddress,  // setter
               hasStagingServerAddress,  // has
               "stagingServerAddress",   // key
               envOrDefault("MVPN_API_BASE_URL",
                            Constants::API_PRODUCTION_URL),  // default value
               false  // remove when reset
)

SETTING_BOOL(stagingServer,     // getter
             setStagingServer,  // setter
             hasStagingServer,  // has
             "stagingServer",   // key
             false,             // default value
             false              // remove when reset
)

SETTING_BOOL(startAtBoot,     // getter
             setStartAtBoot,  // setter
             hasStartAtBoot,  // has
             "startAtBoot",   // key
             false,           // default value
             false            // remove when reset
)

SETTING_BYTEARRAY(surveys,     // getter
                  setSurveys,  // setter
                  hasSurveys,  // has
                  "surveys",   // key
                  "",          // default value
                  true         // remove when reset
)

SETTING_BOOL(systemLanguageCodeMigrated,     // getter
             setSystemLanguageCodeMigrated,  // setter
             hasSystemLanguageCodeMigrated,  // has
             "systemLanguageCodeMigrated",   // key
             false,                          // default value
             true                            // remove when reset
)

SETTING_BOOL(telemetryPolicyShown,     // getter
             setTelemetryPolicyShown,  // setter
             hasTelemetryPolicyShown,  // has
             "telemetryPolicyShown",   // key
             false,                    // default value
             false                     // remove when reset
)

SETTING_STRING(token,     // getter
               setToken,  // setter
               hasToken,  // has
               "token",   // key
               "",        // default value
               true       // remove when reset
)

SETTING_BOOL(unsecuredNetworkAlert,     // getter
             setUnsecuredNetworkAlert,  // setter
             hasUnsecuredNetworkAlert,  // has
             "unsecuredNetworkAlert",   // key
             FeatureUnsecuredNetworkNotification::instance()
                 ->isSupported(),  // default value
             false                 // remove when reset
)

SETTING_STRING(userAvatar,     // getter
               setUserAvatar,  // setter
               hasUserAvatar,  // has
               "user/avatar",  // key
               "",             // default value
               true            // remove when reset
)

SETTING_STRING(userDisplayName,     // getter
               setUserDisplayName,  // setter
               hasUserDisplayName,  // has
               "user/displayName",  // key
               "",                  // default value
               true                 // remove when reset
)

SETTING_STRING(userDNS,     // getter
               setUserDNS,  // setter
               hasUserDNS,  // has
               "userDNS",   // key
               "",          // default value
               false        // remove when reset
)

SETTING_STRING(userEmail,     // getter
               setUserEmail,  // setter
               hasUserEmail,  // has
               "user/email",  // key
               "",            // default value
               true           // remove when reset
)

SETTING_INT(userMaxDevices,     // getter
            setUserMaxDevices,  // setter
            hasUserMaxDevices,  // has
            "user/maxDevices",  // key
            0,                  // default value
            true                // remove when reset
)

SETTING_BOOL(userSubscriptionNeeded,     // getter
             setUserSubscriptionNeeded,  // setter
             hasUserSubscriptionNeeded,  // has
             "user/subscriptionNeeded",  // key
             false,                      // default value
             true                        // remove when reset
)

SETTING_STRINGLIST(vpnDisabledApps,     // getter
                   setVpnDisabledApps,  // setter
                   hasVpnDisabledApps,  // has
                   "vpnDisabledApps",   // key
                   QStringList(),       // default value
                   false                // remove when reset
)

#ifdef MVPN_ANDROID
SETTING_BOOL(nativeAndroidDataMigrated,     // getter
             setNativeAndroidDataMigrated,  // setter
             hasNativeAndroidDataMigrated,  // has
             "nativeAndroidDataMigrated",   // key
             false,                         // default value
             false                          // remove when reset
)
#endif

#ifdef MVPN_WINDOWS
SETTING_BOOL(nativeWindowsDataMigrated,     // getter
             setNativeWindowsDataMigrated,  // setter
             hasNativeWindowsDataMigrated,  // has
             "nativeWindowsDataMigrated",   // key
             false,                         // default value
             false                          // remove when reset
)
#endif

#ifdef MVPN_IOS
SETTING_BOOL(nativeIOSDataMigrated,     // getter
             setNativeIOSDataMigrated,  // setter
             hasNativeIOSDataMigrated,  // has
             "nativeIOSDataMigrated",   // key
             false,                     // default value
             false                      // remove when reset
)

SETTING_STRINGLIST(subscriptionTransactions,     // getter
                   setSubscriptionTransactions,  // setter
                   hasSubscriptionTransactions,  // has
                   "subscriptionTransactions",   // key
                   QStringList(),                // efault value
                   false                         // remove when reset
)
#endif
