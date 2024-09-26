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
        byPlatform({
            .android = true,
            .ios = true,
        }))

FEATURE(addonSignature,             // Feature ID
        "Addons Signature",         // Feature name
        FeatureCallback_inStaging,  // Can be flipped on
        FeatureCallback_inStaging,  // Can be flipped off
        QStringList(),              // feature dependencies
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
        byPlatform(
            {.windows = true, .macos = true, .gnu_linux = true, .wasm = true}))

FEATURE(appReview,              // Feature ID
        "App Review",           // Feature name
        FeatureCallback_false,  // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        byPlatform({
            .android = true,
            .ios = true,
        }))

FEATURE(captivePortal,          // Feature ID
        "Captive Portal",       // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_captivePortal)

FEATURE(checkConnectivityOnActivation,       // Feature ID
        "Check Connectivity On Activation",  // Feature name
        FeatureCallback_true,                // Can be flipped on
        FeatureCallback_true,                // Can be flipped off
        QStringList(),                       // feature dependencies
        FeatureCallback_false)

FEATURE(customDNS,              // Feature ID
        "Custom DNS",           // Feature name
        FeatureCallback_true,   // Can be flipped on
        FeatureCallback_false,  // Can be flipped off
        QStringList(),          // feature dependencies
        FeatureCallback_true)

FEATURE(enableUpdateServer,         // Feature ID
        "Enable Update Server",     // Feature name
        FeatureCallback_hasBalrog,  // Can be flipped on
        FeatureCallback_hasBalrog,  // Can be flipped off
        QStringList(),              // feature dependencies
        FeatureCallback_hasBalrog)

FEATURE(factoryReset,          // Feature ID
        "Factory reset",       // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
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
        FeatureCallback_inAppAuthentication)

FEATURE(inAppAuthentication,      // Feature ID
        "In-app Authentication",  // Feature name
        FeatureCallback_true,     // Can be flipped on
        FeatureCallback_true,     // Can be flipped off
        QStringList(),            // feature dependencies
        FeatureCallback_inAppAuthentication)

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
        byPlatform(
            {.windows = true, .macos = true, .gnu_linux = true, .wasm = true}))

FEATURE(stagingUpdateServer,        // Feature ID
        "Staging Update Server",    // Feature name
        FeatureCallback_hasBalrog,  // Can be flipped on
        FeatureCallback_false,      // Can be flipped off
        QStringList(),              // feature dependencies
        FeatureCallback_false)

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

FEATURE(unsecuredNetworkNotification,      // Feature ID
        "Unsecured network notification",  // Feature name
        FeatureCallback_true,              // Can be flipped on
        FeatureCallback_false,             // Can be flipped off
        QStringList(),                     // feature dependencies
        byPlatform(
            {.windows = true, .macos = true, .gnu_linux = true, .wasm = true}))

FEATURE(webPurchase,           // Feature ID
        "Web Purchase",        // Feature name
        FeatureCallback_true,  // Can be flipped on
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        byPlatform({
            .windows = true,
            .macos = true,
            .gnu_linux = true,
        }))

FEATURE(localProxy,    // Feature ID
        "LocalProxy",  // Feature name
        byPlatform({
            .windows = true,
            .gnu_linux = true,
        }),                          // Can be flipped on
        FeatureCallback_true,        // Can be flipped off
        QStringList("splitTunnel"),  // feature dependencies
        FeatureCallback_inStaging)

FEATURE(webExtension,    // Feature ID
        "webExtension",  // Feature name
        byPlatform({     // Can be flipped on
                    .windows = true,
                    .macos = true,
                    .gnu_linux = true}),
        FeatureCallback_true,  // Can be flipped off
        QStringList(),         // feature dependencies
        byPlatform({           // default value
                    .windows = true}))
