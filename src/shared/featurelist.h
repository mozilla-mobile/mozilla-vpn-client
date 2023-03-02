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

// The app must implement its feature list file.
#include "appfeaturelist.h"
