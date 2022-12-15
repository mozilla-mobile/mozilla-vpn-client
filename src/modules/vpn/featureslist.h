/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file is used to include the feature list for the VPN module
// NOTE! do not include this file directly. Use featureslist.h instead.

FEATURE_SIMPLE(benchmarkUpload,       // Feature ID
               "Benchmark Upload",    // Feature name
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

FEATURE_SIMPLE(keyRegeneration,       // Feature ID
               "Key Regeneration",    // Feature name
               "2.10.0",              // released
               FeatureCallback_true,  // Can be flipped on
               FeatureCallback_true,  // Can be flipped off
               QStringList(),         // feature dependencies
               // TODO: Disabled - VPN-2693
               FeatureCallback_false)

FEATURE_SIMPLE(lanAccess,                    // Feature ID
               "Local area network access",  // Feature name
               "2.2",                        // released
               FeatureCallback_true,         // Can be flipped on
               FeatureCallback_false,        // Can be flipped off
               QStringList(),                // feature dependencies
               FeatureCallback_lanAccess)

#if defined(MVPN_ANDROID) || defined(MVPN_IOS)
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

FEATURE_SIMPLE(recommendedServers,     // Feature ID
               "Recommended servers",  // Feature name
               "2.12",                 // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_true,   // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_false)

FEATURE_SIMPLE(splitTunnel,            // Feature ID
               "Split-tunnel",         // Feature name
               "2.4",                  // released
               FeatureCallback_true,   // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_splitTunnel)

FEATURE_SIMPLE(unsecuredNetworkNotification,      // Feature ID
               "Unsecured network notification",  // Feature name
               "2.2",                             // released
               FeatureCallback_true,              // Can be flipped on
               FeatureCallback_false,             // Can be flipped off
               QStringList(),                     // feature dependencies
               FeatureCallback_unsecuredNetworkNotification)
