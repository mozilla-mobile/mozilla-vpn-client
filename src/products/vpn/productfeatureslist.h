/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file is used to include the feature list for this product and its
// modules.

//  NOTE! do not include this file directly. Use featureslist.h instead.

#include "modules/vpn/featureslist.h"

FEATURE_SIMPLE(accountDeletion,        // Feature ID
               "Account deletion",     // Feature name
               "2.9",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_accountDeletion)

FEATURE_SIMPLE(bundleUpgrade,         // Feature ID
               "Bundle Upgrade",      // Feature name
               "2.10",                // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_false)

FEATURE_SIMPLE(freeTrial,             // Feature ID
               "Free trial",          // Feature name
               "2.8.1",               // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               FeatureCallback_freeTrial)

FEATURE_SIMPLE(mobileOnboarding,       // Feature ID
               "Mobile Onboarding",    // Feature name
               "2.8",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE_SIMPLE(notificationControl,     // Feature ID
               "Notification control",  // Feature name
               "2.3",                   // released
               FeatureCallback_true,    // Can be flipped on
               FeatureCallback_false,   // Can be flipped off
               QStringList(),           // feature dependencies
               FeatureCallback_true)
