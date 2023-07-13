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

// FEATURE(addon,                 // Feature ID
//         "Addon support",       // Feature name
//         FeatureCallback_true,  // Can be flipped on
//         FeatureCallback_true,  // Can be flipped off
//         QStringList(),         // feature dependencies
//         FeatureCallback_true)

// FEATURE(addonSignature,             // Feature ID
//         "Addons Signature",         // Feature name
//         FeatureCallback_inStaging,  // Can be flipped on
//         FeatureCallback_inStaging,  // Can be flipped off
//         QStringList{"addon"},       // feature dependencies
//         FeatureCallback_true)

// FEATURE(appReview,              // Feature ID
//         "App Review",           // Feature name
//         FeatureCallback_false,  // Can be flipped on
//         FeatureCallback_false,  // Can be flipped off
//         QStringList(),          // feature dependencies
//         FeatureCallback_iosOrAndroid)

// FEATURE(gleanRust,             // Feature ID
//         "Glean Rust SDK",      // Feature name
//         FeatureCallback_true,  // Can be flipped on
//         FeatureCallback_true,  // Can be flipped off
//         QStringList(),         // feature dependencies
//         FeatureCallback_true)

// FEATURE(inAppAccountCreate,                  // Feature ID
//         "In-app Account Creation",           // Feature name
//         FeatureCallback_true,                // Can be flipped on
//         FeatureCallback_true,                // Can be flipped off
//         QStringList{"inAppAuthentication"},  // feature dependencies
//         FeatureCallback_true)

// FEATURE(inAppAuthentication,      // Feature ID
//         "In-app Authentication",  // Feature name
//         FeatureCallback_true,     // Can be flipped on
//         FeatureCallback_true,     // Can be flipped off
//         QStringList(),            // feature dependencies
//         FeatureCallback_true)

// FEATURE(sentry,                     // Feature ID
//         "Sentry Crash Report SDK",  // Feature name
//         FeatureCallback_true,       // Can be flipped on
//         FeatureCallback_true,       // Can be flipped off
//         QStringList(),              // feature dependencies
//         FeatureCallback_true)

// FEATURE(shareLogs,              // Feature ID
//         "Share Logs",           // Feature name
//         FeatureCallback_false,  // Can be flipped on
//         FeatureCallback_false,  // Can be flipped off
//         QStringList(),          // feature dependencies
//         FeatureCallback_shareLogs)

// FEATURE(webPurchase,           // Feature ID
//         "Web Purchase",        // Feature name
//         FeatureCallback_true,  // Can be flipped on
//         FeatureCallback_true,  // Can be flipped off
//         QStringList(),         // feature dependencies
//         FeatureCallback_webPurchase)

// FEATURE(replacerAddon,         // Feature ID
//         "Replacer Addon",      // Feature name
//         FeatureCallback_true,  // Can be flipped on
//         FeatureCallback_true,  // Can be flipped off
//         QStringList(),         // feature dependencies
//         FeatureCallback_false)

// The app must implement its feature list file.
#include "appfeaturelist.h"
