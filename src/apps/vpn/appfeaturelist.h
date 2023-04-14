/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE! Do not include this file directly. Use featurelist.h instead.

FEATURE(accountDeletion,        // Feature ID
        "Account deletion",     // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_accountDeletion)

FEATURE(alwaysPort53,          // Feature ID
        "Always use port 53",  // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_false)

FEATURE(benchmarkUpload,       // Feature ID
        "Benchmark Upload",    // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_true)

FEATURE(bundleUpgrade,         // Feature ID
        "Bundle Upgrade",      // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_false)

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

FEATURE(freeTrial,             // Feature ID
        "Free trial",          // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_freeTrial)

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

FEATURE(unsecuredNetworkNotification,      // Feature ID
        "Unsecured network notification",  // Feature name
        FeatureCallback_true,              // Can be flipped on
        FeatureCallback_false,             // Can be flipped off
        QStringList(),                     // feature dependencies
        FeatureCallback_unsecuredNetworkNotification)

FEATURE(websocket,             // Feature ID
        "WebSocket",           // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        FeatureCallback_true)
