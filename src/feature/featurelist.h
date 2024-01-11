/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// 1. Define the FEATURE macro
// 2. include this file
// 3. undefine the FEATURE macro

// If you want to use the callback, include `featurelistcallback.h`

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#  error Not supported
#endif

FEATURE(accountDeletion,        // Feature ID
        "Account deletion",     // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_iosOrAndroid)

FEATURE(addon,                 // Feature ID
        "Addon support",       // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_true)

FEATURE(addonSignature,             // Feature ID
        "Addons Signature",         // Feature name
        FeatureCallback_inStaging,  // Can be flipped on
        FeatureCallback_inStaging,  // Can be flipped off
        QStringList{"addon"},       // feature dependencies
        FeatureCallback_true)

FEATURE(alwaysPort53,          // Feature ID
        "Always use port 53",  // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_false)

FEATURE(annualUpgrade,         // Feature ID
        "Annual upgrade",      // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_annualUpgrade)

FEATURE(appReview,              // Feature ID
        "App Review",           // Feature name
        FeatureCallback_false,  // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_iosOrAndroid)

FEATURE(benchmarkUpload,       // Feature ID
        "Benchmark Upload",    // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_true)

FEATURE(captivePortal,          // Feature ID
        "Captive Portal",       // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_captivePortal)

FEATURE(customDNS,              // Feature ID
        "Custom DNS",           // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_true)

FEATURE(freeTrial,              // Feature ID
        "Free trial",           // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_true,   // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_false)  // Disabled while we rethink free trials

FEATURE(gleanRust,             // Feature ID
        "Glean Rust SDK",      // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_true)

FEATURE(inAppAccountCreate,                  // Feature ID
        "In-app Account Creation",           // Feature name
        FeatureCallback_true,                // Can be flipped on
        FeatureCallback_true,                // Can be flipped off
        QStringList{"inAppAuthentication"},  // feature dependencies
        FeatureCallback_true)

FEATURE(inAppAuthentication,      // Feature ID
        "In-app Authentication",  // Feature name
        FeatureCallback_true,     // Can be flipped on
        FeatureCallback_true,     // Can be flipped off
        QStringList(),            // feature dependencies
        FeatureCallback_true)

FEATURE(keyRegeneration,       // Feature ID
        "Key Regeneration",    // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        // TODO: Disabled - VPN-2693
        FeatureCallback_false)

FEATURE(multiHop,               // Feature ID
        "Multi-hop",            // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_true)

FEATURE(newOnboarding,         // Feature ID
        "New onboarding",      // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_false)

FEATURE(notificationControl,     // Feature ID
        "Notification control",  // Feature name
        FeatureCallback_true,    // Can be flipped on
        FeatureCallback_false,   // Can be flipped off
        QStringList(),           // feature dependencies
        FeatureCallback_true)

FEATURE(recommendedServers,     // Feature ID
        "Recommended servers",  // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_true,   // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_true)

FEATURE(replacerAddon,         // Feature ID
        "Replacer Addon",      // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_false)

FEATURE(sentry,                     // Feature ID
        "Sentry Crash Report SDK",  // Feature name
        FeatureCallback_true,       // Can be flipped on
        FeatureCallback_true,       // Can be flipped off
        QStringList(),              // feature dependencies
        FeatureCallback_sentry)

FEATURE(serverUnavailableNotification,      // Feature ID
        "Server unavailable notification",  // Feature name
        FeatureCallback_true,               // Can be flipped on
        FeatureCallback_false,              // Can be flipped off
        QStringList(),                      // feature dependencies
        FeatureCallback_true)

FEATURE(serverConnectionScore,      // Feature ID
        "Server connection score",  // Feature name
        FeatureCallback_true,       // Can be flipped on
        FeatureCallback_true,       // Can be flipped off
        QStringList(),              // feature dependencies
        FeatureCallback_true)

FEATURE(shareLogs,              // Feature ID
        "Share Logs",           // Feature name
        FeatureCallback_false,  // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_shareLogs)

FEATURE(splitTunnel,            // Feature ID
        "Split-tunnel",         // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_splitTunnel)

FEATURE(startOnBoot,            // Feature ID
        "Start on boot",        // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_startOnBoot)

FEATURE(subscriptionManagement,     // Feature ID
        "Subscription management",  // Feature name
        FeatureCallback_true,       // Can be flipped on
        FeatureCallback_true,       // Can be flipped off
        QStringList(),              // feature dependencies
        FeatureCallback_true)

FEATURE(superDooperMetrics,      // Feature ID
        "Super Dooper Metrics",  // Feature name
        FeatureCallback_true,    // Can be flipped on
        FeatureCallback_true,    // Can be flipped off
        QStringList(),           // feature dependencies
        FeatureCallback_true)

FEATURE(swipeToGoBack,      // Feature ID
        "Swipe to go back",  // Feature name
        FeatureCallback_true,    // Can be flipped on
        FeatureCallback_true,    // Can be flipped off
        QStringList(),           // feature dependencies
        FeatureCallback_swipeToGoBack)

FEATURE(unsecuredNetworkNotification,      // Feature ID
        "Unsecured network notification",  // Feature name
        FeatureCallback_true,              // Can be flipped on
        FeatureCallback_false,             // Can be flipped off
        QStringList(),                     // feature dependencies
        FeatureCallback_unsecuredNetworkNotification)

FEATURE(webPurchase,           // Feature ID
        "Web Purchase",        // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_webPurchase)
