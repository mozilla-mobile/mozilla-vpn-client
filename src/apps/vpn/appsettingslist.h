/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE! Do not include this file directly. Include settingslist.h instead.

// Please! Keep the alphabetic order!

SETTING_BOOL(captivePortalAlert,        // getter
             setCaptivePortalAlert,     // setter
             removeCaptivePortalAlert,  // remover
             hasCaptivePortalAlert,     // has
             "captivePortalAlert",      // key
             Feature::get(Feature::Feature_captivePortal)
                 ->isSupported(),  // default value
             true,                 // user setting
             false,                // remove when reset
             false                 // sensitive (do not log)
)

SETTING_STRINGLIST(captivePortalIpv4Addresses,        // getter
                   setCaptivePortalIpv4Addresses,     // setter
                   removeCaptivePortalIpv4Addresses,  // remover
                   hasCaptivePortalIpv4Addresses,     // has
                   "captivePortal/ipv4Addresses",     // key
                   QStringList(),                     // default value
                   false,                             // user setting
                   false,                             // remove when reset
                   false                              // sensitive (do not log)
)

SETTING_STRINGLIST(captivePortalIpv6Addresses,         // getter
                   setCaptivePortalIpv6Addresses,      // setter
                   removerCaptivePortalIpv6Addresses,  // remover
                   hasCaptivePortalIpv6Addresses,      // has
                   "captivePortal/ipv6Addresses",      // key
                   QStringList(),                      // default value
                   false,                              // user setting
                   false,                              // remove when reset
                   false                               // sensitive (do not log)
)

SETTING_BOOL(connectionChangeNotification,        // getter
             setConnectionChangeNotification,     // setter
             removeConnectionChangeNotification,  // remover
             hasConnectionChangeNotification,     // has
             "connectionChangeNotification",      // key
             true,                                // default value
             true,                                // user setting
             false,                               // remove when reset
             false                                // sensitive (do not log)
)

SETTING_STRING(currentServerCityDeprecated,        // getter
               setCurrentServerCityDeprecated,     // setter
               removeCurrentServerCityDeprecated,  // remover
               hasCurrentServerCityDeprecated,     // has
               "currentServer/city",               // key
               "",                                 // default value
               true,                               // user setting
               true,                               // remove when reset
               false                               // sensitive (do not log)
)

SETTING_STRING(currentServerCountryCodeDeprecated,        // getter
               setCurrentServerCountryCodeDeprecated,     // setter
               removeCurrentServerCountryCodeDeprecated,  // remover
               hasCurrentServerCountryCodeDeprecated,     // has
               "currentServer/countryCode",               // key
               "",                                        // default value
               true,                                      // user setting
               true,                                      // remove when reset
               false  // sensitive (do not log)
)

SETTING_BOOL(developerUnlock,        // getter
             setDeveloperUnlock,     // setter
             removeDeveloperUnlock,  // remover
             hasDeveloperUnlock,     // has
             "developerUnlock",      // key
             false,                  // default value
             false,                  // user setting
             false,                  // remove when reset
             false                   // sensitive (do not log)
)

SETTING_STRING(deviceKeyVersion,        // getter
               setDeviceKeyVersion,     // setter
               removeDeviceKeyVersion,  // remover
               hasDeviceKeyVersion,     // has
               "deviceKeyVersion",      // key
               "",                      // default value
               false,                   // user setting
               true,                    // remove when reset
               false                    // sensitive (do not log)
)

SETTING_BYTEARRAY(devices,        // getter
                  setDevices,     // setter
                  removeDevices,  // remover
                  hasDevices,     // has
                  "devices",      // key
                  "",             // default value
                  false,          // user setting
                  true,           // remove when reset
                  true  // sensitive (do not log) - noisy and limited value
)

SETTING_INT(dnsProviderDeprecated,                      // getter
            setDNSProviderDeprecated,                   // setter
            removeDNSProviderDeprecated,                // remover
            hasDNSProviderDeprecated,                   // has
            "dnsProvider",                              // key
            SettingsHolder::DNSProviderFlags::Gateway,  // default value
            true,                                       // user setting
            false,                                      // remove when reset
            false  // sensitive (do not log)
)

SETTING_INT(dnsProviderFlags,                           // getter
            setDNSProviderFlags,                        // setter
            removeDNSProviderFlags,                     // remover
            hasDNSProviderFlags,                        // has
            "dnsProviderFlags",                         // key
            SettingsHolder::DNSProviderFlags::Gateway,  // default value
            true,                                       // user setting
            false,                                      // remove when reset
            false  // sensitive (do not log)
)

SETTING_STRING(entryServerCityDeprecated,        // getter
               setEntryServerCityDeprecated,     // setter
               removeEntryServerCityDeprecated,  // remover
               hasEntryServerCityDeprecated,     // has
               "entryServer/city",               // key
               nullptr,                          // default value
               true,                             // user setting
               true,                             // remove when reset
               false                             // sensitive (do not log)
)

SETTING_STRING(entryServerCountryCodeDeprecated,        // getter
               setEntryServerCountryCodeDeprecated,     // setter
               removeEntryServerCountryCodeDeprecated,  // remover
               hasEntryServerCountryCodeDeprecated,     // has
               "entryServer/countryCode",               // key
               nullptr,                                 // default value
               true,                                    // user setting
               true,                                    // remove when reset
               false  // sensitive (do not log)
)

SETTING_STRINGLIST(iapProducts,        // getter
                   setIapProducts,     // setter
                   removeIapProducts,  // remover
                   hasIapProducts,     // has
                   "iapProducts",      // key
                   QStringList(),      // default value
                   false,              // user setting
                   true,               // remove when reset
                   false               // sensitive (do not log)
)

SETTING_INT64(keyRegenerationTimeSec,        // getter
              setKeyRegenerationTimeSec,     // setter
              removeKeyRegenerationTimeSec,  // remover
              hasKeyRegenerationTimeSec,     // has
              "keyRegenerationTimeSec",      // key
              0,                             // default value
              false,                         // user setting
              true,                          // remove when reset
              false                          // sensitive (do not log)
)

SETTING_STRINGLIST(missingApps,        // getter
                   setMissingApps,     // setter
                   removeMissingApps,  // remover
                   hasMissingApps,     // has
                   "MissingApps",      // key
                   QStringList(),      // default value
                   true,               // user setting
                   false,              // remove when reset
                   false               // sensitive (do not log)
)

SETTING_BOOL(postAuthenticationShown,        // getter
             setPostAuthenticationShown,     // setter
             removePostAuthenticationShown,  // remover
             hasPostAuthenticationShown,     // has
             "postAuthenticationShown",      // key
             false,                          // default value
             true,                           // user setting
             true,                           // remove when reset
             false                           // sensitive (do not log)
)

SETTING_STRING(privateKey,        // getter
               setPrivateKey,     // setter
               removePrivateKey,  // remover
               hasPrivateKey,     // has
               "privateKey",      // key
               "",                // default value
               false,             // user setting
               true,              // remove when reset
               true               // sensitive (do not log)
)

SETTING_STRING(privateKeyJournal,        // getter
               setPrivateKeyJournal,     // setter
               removePrivateKeyJournal,  // remover
               hasPrivateKeyJournal,     // has
               "privateKeyJournal",      // key
               "",                       // default value
               false,                    // user setting
               true,                     // remove when reset
               true                      // sensitive (do not log)
)

SETTING_BOOL(protectSelectedApps,        // getter
             setProtectSelectedApps,     // setter
             removeProtectSelectedApps,  // remover
             hasProtectSelectedApps,     // has
             "protectSelectedApps",      // key
             false,                      // default value
             true,                       // user setting
             false,                      // remove when reset
             false                       // sensitive (do not log)
)

SETTING_STRING(publicKey,        // getter
               setPublicKey,     // setter
               removePublicKey,  // remover
               hasPublicKey,     // has
               "publicKey",      // key
               "",               // default value
               false,            // user setting
               true,             // remove when reset
               true              // sensitive (do not log)
)

SETTING_STRING(publicKeyJournal,        // getter
               setPublicKeyJournal,     // setter
               removePublicKeyJournal,  // remover
               hasPublicKeyJournal,     // has
               "publicKeyJournal",      // key
               "",                      // default value
               false,                   // user setting
               true,                    // remove when reset
               true                     // sensitive (do not log)
)

SETTING_BYTEARRAY(recentConnections,        // getter
                  setRecentConnections,     // setter
                  removeRecentConnections,  // remover
                  hasRecentConnections,     // has
                  "recentConnections2",     // key
                  "",                       // default value
                  true,                     // user setting
                  true,                     // remove when reset
                  true                      // sensitive (do not log)
)

SETTING_STRINGLIST(recentConnectionsDeprecated,        // getter
                   setRecentConnectionsDeprecated,     // setter
                   removeRecentConnectionsDeprecated,  // remover
                   hasRecentConnectionsDeprecated,     // has
                   "recentConnections",                // key
                   QStringList(),                      // default value
                   true,                               // user setting
                   true,                               // remove when reset
                   true                                // sensitive (do not log)
)

SETTING_BYTEARRAY(servers,        // getter
                  setServers,     // setter
                  removeServers,  // remover
                  hasServers,     // has
                  "servers",      // key
                  "",             // default value
                  false,          // user setting
                  true,           // remove when reset
                  true  // sensitive (do not log) - noisy and limited value
)

SETTING_BYTEARRAY(serverData,        // getter
                  setServerData,     // setter
                  removeServerData,  // remover
                  hasServerData,     // has
                  "serverData",      // key
                  "",                // default value
                  true,              // user setting
                  true,              // remove when reset
                  true               // sensitive (do not log)
)

SETTING_BOOL(serverSwitchNotification,        // getter
             setServerSwitchNotification,     // setter
             removeServerSwitchNotification,  // remover
             hasServerSwitchNotification,     // has
             "serverSwitchNotification",      // key
             true,                            // default value
             true,                            // user setting
             false,                           // remove when reset
             false                            // sensitive (do not log)
)

SETTING_BOOL(serverUnavailableNotification,        // getter
             setServerUnavailableNotification,     // setter
             removeServerUnavailableNotification,  // remover
             hasServerUnavailableNotification,     // has
             "serverUnavailableNotification",      // key
             Feature::get(Feature::Feature_serverUnavailableNotification)
                 ->isSupported(),  // default value
             true,                 // user setting
             false,                // remove when reset
             false                 // sensitive (do not log)
)

SETTING_BOOL(recommendedServerSelected,        // getter
             setRecommendedServerSelected,     // setter
             removeRecommendedServerSelected,  // remover
             hasRecommendedServerSelected,     // has
             "recommendedServerSelected",      // key
             true,                             // default value
             false,                            // user setting
             false,                            // remove when reset
             false                             // sensitive (do not log)
)

SETTING_STRING(
    stagingServerAddress,        // getter
    setStagingServerAddress,     // setter
    removeStagingServerAddress,  // remover
    hasStagingServerAddress,     // has
    "stagingServerAddress",      // key
    Constants::envOrDefault("MVPN_API_BASE_URL",
                            AppConstants::API_STAGING_URL),  // default value
    false,                                                   // user setting
    false,  // remove when reset
    false   // sensitive (do not log)
)

#ifdef MZ_WASM
#  define STAGING_SERVER_DEFAULT_VALUE true
#else
#  define STAGING_SERVER_DEFAULT_VALUE false
#endif
SETTING_BOOL(stagingServer,                 // getter
             setStagingServer,              // setter
             removeStagingServer,           // remover
             hasStagingServer,              // has
             "stagingServer",               // key
             STAGING_SERVER_DEFAULT_VALUE,  // default value
             false,                         // user setting
             false,                         // remove when reset
             false                          // sensitive (do not log)
)

SETTING_BOOL(startAtBoot,        // getter
             setStartAtBoot,     // setter
             removeStartAtBoot,  // remover
             hasStartAtBoot,     // has
             "startAtBoot",      // key
             false,              // default value
             true,               // user setting
             false,              // remove when reset
             false               // sensitive (do not log)
)

SETTING_BYTEARRAY(subscriptionData,        // getter
                  setSubscriptionData,     // setter
                  removeSubscriptionData,  // remover
                  hasSubscriptionData,     // has
                  "subscriptionData",      // key
                  "",                      // default value
                  false,                   // user setting
                  true,                    // remove when reset
                  true                     // sensitive (do not log)
)

SETTING_BOOL(telemetryPolicyShown,        // getter
             setTelemetryPolicyShown,     // setter
             removeTelemetryPolicyShown,  // remover
             hasTelemetryPolicyShown,     // has
             "telemetryPolicyShown",      // key
             false,                       // default value
             false,                       // user setting
             false,                       // remove when reset
             false                        // sensitive (do not log)
)

SETTING_BOOL(tipsAndTricksIntroShown,        // getter
             setTipsAndTricksIntroShown,     // setter
             removeTipsAndTricksIntroShown,  // remover
             hasTipsAndTricksIntroShown,     // has
             "tipsAndTricksIntroShown",      // key
             false,                          // default value
             false,                          // user setting
             false,                          // remove when reset
             false                           // sensitive (do not log)t
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

SETTING_BOOL(unsecuredNetworkAlert,        // getter
             setUnsecuredNetworkAlert,     // setter
             removeUnsecuredNetworkAlert,  // remover
             hasUnsecuredNetworkAlert,     // has
             "unsecuredNetworkAlert",      // key
             Feature::get(Feature::Feature_unsecuredNetworkNotification)
                 ->isSupported(),  // default value
             true,                 // user setting
             false,                // remove when reset
             false                 // sensitive (do not log)
)

SETTING_STRING(userAvatar,        // getter
               setUserAvatar,     // setter
               removeUserAvatar,  // remover
               hasUserAvatar,     // has
               "user/avatar",     // key
               "",                // default value
               false,             // user setting
               true,              // remove when reset
               true               // sensitive (do not log)
)

SETTING_STRING(userDisplayName,        // getter
               setUserDisplayName,     // setter
               removeUserDisplayName,  // remover
               hasUserDisplayName,     // has
               "user/displayName",     // key
               "",                     // default value
               false,                  // user setting
               true,                   // remove when reset
               true                    // sensitive (do not log)
)

SETTING_STRING(userDNS,        // getter
               setUserDNS,     // setter
               removeUserDNS,  // remover
               hasUserDNS,     // has
               "userDNS",      // key
               "",             // default value
               true,           // user setting
               false,          // remove when reset
               false           // sensitive (do not log)
)

SETTING_INT(userMaxDevices,        // getter
            setUserMaxDevices,     // setter
            removeUserMaxDevices,  // remover
            hasUserMaxDevices,     // has
            "user/maxDevices",     // key
            0,                     // default value
            false,                 // user setting
            true,                  // remove when reset
            false                  // sensitive (do not log)
)

SETTING_BOOL(userSubscriptionNeeded,        // getter
             setUserSubscriptionNeeded,     // setter
             removeUserSubscriptionNeeded,  // remover
             hasUserSubscriptionNeeded,     // has
             "user/subscriptionNeeded",     // key
             false,                         // default value
             false,                         // user setting
             true,                          // remove when reset
             false                          // sensitive (do not log)
)

SETTING_STRINGLIST(vpnDisabledApps,        // getter
                   setVpnDisabledApps,     // setter
                   removeVpnDisabledApps,  // remover
                   hasVpnDisabledApps,     // has
                   "vpnDisabledApps",      // key
                   QStringList(),          // default value
                   true,                   // user setting
                   false,                  // remove when reset
                   false                   // sensitive (do not log)
)

#if defined(MVPN_ADJUST)
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

#if defined(MZ_IOS)
SETTING_STRINGLIST(subscriptionTransactions,        // getter
                   setSubscriptionTransactions,     // setter
                   removeSubscriptionTransactions,  // remover
                   hasSubscriptionTransactions,     // has
                   "subscriptionTransactions",      // key
                   QStringList(),                   // efault value
                   false,                           // user setting
                   false,                           // remove when reset
                   true                             // sensitive (do not log)
)
#endif
