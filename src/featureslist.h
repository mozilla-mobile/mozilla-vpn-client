/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// 1. Define the FEATURE macro
// 2. include this file
// 3. undefine the FEATURE macro

// If you want to use the callback, include `featurelistcallback.h`

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#  error No supported
#endif

#define FEATURE_SIMPLE(id, name, releaseVersion, flippableOn, flippableOff, \
                       dependencies, callback)                              \
  FEATURE(id, name, false, L18nStrings::Empty, L18nStrings::Empty,          \
          L18nStrings::Empty, "", "", "", releaseVersion, flippableOn,      \
          flippableOff, dependencies, callback)

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
               FeatureCallback_false)

FEATURE_SIMPLE(sentry,                     // Feature ID
               "Sentry Crash Report SDK",  // Feature name
               "2.12.0",                   // released
               FeatureCallback_true,       // Can be flipped on
               FeatureCallback_true,       // Can be flipped off
               QStringList(),              // feature dependencies
               FeatureCallback_inStaging)

#include "productfeatureslist.h"
