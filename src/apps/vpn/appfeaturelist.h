/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE! Do not include this file directly. Use featurelist.h instead.

FEATURE_SIMPLE(accountDeletion,        // Feature ID
               "Account deletion",     // Feature name
               "2.9",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_accountDeletion)

FEATURE_SIMPLE(addon,                 // Feature ID
               "Addon support",       // Feature name
               "2.9.0",               // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(addonSignature,             // Feature ID
               "Addons Signature",         // Feature name
               "2.10.0",                   // released
               FeatureCallback_inStaging,  // Can be flipped on
               FeatureCallback_inStaging,  // Can be flipped off
               QStringList{"addon"},       // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(appReview,              // Feature ID
               "App Review",           // Feature name
               "2.5",                  // released
               FeatureCallback_false,  // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE_SIMPLE(benchmarkUpload,       // Feature ID
               "Benchmark Upload",    // Feature name
               "2.13",                // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(bundleUpgrade,         // Feature ID
               "Bundle Upgrade",      // Feature name
               "2.10",                // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_false)

FEATURE_SIMPLE(captivePortal,          // Feature ID
               "Captive Portal",       // Feature name
               "2.1",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_captivePortal)

FEATURE_SIMPLE(customDNS,              // Feature ID
               "Custom DNS",           // Feature name
               "2.5",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(freeTrial,             // Feature ID
               "Free trial",          // Feature name
               "2.8.1",               // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_freeTrial)

FEATURE_SIMPLE(inAppAccountCreate,                  // Feature ID
               "In-app Account Creation",           // Feature name
               "2.6",                               // released
               FeatureCallback_true,                // Can be flipped on
               FeatureCallback_true,                // Can be flipped off
               QStringList{"inAppAuthentication"},  // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(inAppAuthentication,      // Feature ID
               "In-app Authentication",  // Feature name
               "2.4",                    // released
               FeatureCallback_true,     // Can be flipped on
               FeatureCallback_true,     // Can be flipped off
               QStringList(),            // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(webPurchase,           // Feature ID
               "Web Purchase",        // Feature name
               "2.12",                // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_webPurchase)

FEATURE_SIMPLE(keyRegeneration,       // Feature ID
               "Key Regeneration",    // Feature name
               "2.10.0",              // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               // TODO: Disabled - VPN-2693
               FeatureCallback_false)

FEATURE_SIMPLE(mobileOnboarding,       // Feature ID
               "Mobile Onboarding",    // Feature name
               "2.8",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_iosOrAndroid)

#if defined(MZ_ANDROID) || defined(MZ_IOS)
#  define MULTIHOP_RELEASE "2.7"
#else
#  define MULTIHOP_RELEASE "2.5"
#endif

FEATURE_SIMPLE(multiHop,               // Feature ID
               "Multi-hop",            // Feature name
               MULTIHOP_RELEASE,       // released for android
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(notificationControl,     // Feature ID
               "Notification control",  // Feature name
               "2.3",                   // released
               FeatureCallback_true,    // Can be flipped on
               FeatureCallback_false,   // Can be flipped off
               QStringList(),           // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(recommendedServers,     // Feature ID
               "Recommended servers",  // Feature name
               "2.12",                 // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_true,   // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_false)

FEATURE_SIMPLE(serverUnavailableNotification,      // Feature ID
               "Server unavailable notification",  // Feature name
               "2.7",                              // released
               FeatureCallback_true,               // Can be flipped on
               FeatureCallback_false,              // Can be flipped off
               QStringList(),                      // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(serverConnectionScore,      // Feature ID
               "Server connection score",  // Feature name
               "2.10",                     // released
               FeatureCallback_true,       // Can be flipped on
               FeatureCallback_true,       // Can be flipped off
               QStringList(),              // feature dependencies
               FeatureCallback_false)

FEATURE_SIMPLE(shareLogs,              // Feature ID
               "Share Logs",           // Feature name
               "2.6",                  // released
               FeatureCallback_false,  // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_shareLogs)

FEATURE_SIMPLE(splitTunnel,            // Feature ID
               "Split-tunnel",         // Feature name
               "2.4",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_splitTunnel)

FEATURE_SIMPLE(startOnBoot,            // Feature ID
               "Start on boot",        // Feature name
               "2.0",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_startOnBoot)

FEATURE_SIMPLE(subscriptionManagement,     // Feature ID
               "Subscription management",  // Feature name
               "2.9",                      // released
               FeatureCallback_true,       // Can be flipped on
               FeatureCallback_true,       // Can be flipped off
               QStringList(),              // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(unsecuredNetworkNotification,      // Feature ID
               "Unsecured network notification",  // Feature name
               "2.2",                             // released
               FeatureCallback_true,              // Can be flipped on
               FeatureCallback_false,             // Can be flipped off
               QStringList(),                     // feature dependencies
               FeatureCallback_unsecuredNetworkNotification)

FEATURE_SIMPLE(websocket,             // Feature ID
               "WebSocket",           // Feature name
               "2.9.0",               // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(gleanRust,             // Feature ID
               "Glean Rust SDK",      // Feature name
               "2.11.0",              // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(sentry,                     // Feature ID
               "Sentry Crash Report SDK",  // Feature name
               "2.12.0",                   // released
               FeatureCallback_true,       // Can be flipped on
               FeatureCallback_true,       // Can be flipped off
               QStringList(),              // feature dependencies
               FeatureCallback_inStaging)
