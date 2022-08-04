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

FEATURE_SIMPLE(accountDeletion,     // Feature ID
               "Account deletion",  // Feature name
               "2.9",               // released
               true,                // Can be flipped on
               false,               // Can be flipped off
               QStringList(),       // feature dependencies
               FeatureCallback_accountDeletion)

FEATURE_SIMPLE(addon,            // Feature ID
               "Addon support",  // Feature name
               "2.9.0",          // released
               true,             // Can be flipped on
               true,             // Can be flipped off
               QStringList(),    // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(appReview,      // Feature ID
               "App Review",   // Feature name
               "2.5",          // released
               false,          // Can be flipped on
               false,          // Can be flipped off
               QStringList(),  // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE_SIMPLE(captivePortal,     // Feature ID
               "Captive Portal",  // Feature name
               "2.1",             // released
               true,              // Can be flipped on
               false,             // Can be flipped off
               QStringList(),     // feature dependencies
               FeatureCallback_captivePortal)

FEATURE(
    connectionInfo,     // Feature ID
    "Connection info",  // Feature name
    true,               // Is Major Feature
    L18nStrings ::WhatsNewReleaseNotesConnectionInfoDisplayName,  // Display
                                                                  // name
    L18nStrings::
        WhatsNewReleaseNotesConnectionInfoDescriptionShort,      // Description
    L18nStrings::WhatsNewReleaseNotesConnectionInfoDescription,  // LongDescr
    "qrc:/ui/resources/features/connection-info-preview.png",    // ImagePath
    "qrc:/ui/resources/features/connection-info-icon.svg",       // IconPath
    "",                                                          // link URL
    "2.8",                                                       // released
    true,           // Can be flipped on
    false,          // Can be flipped off
    QStringList(),  // feature dependencies
    FeatureCallback_true)

FEATURE(customDNS,                                         // Feature ID
        "Custom DNS",                                      // Feature name
        true,                                              // Is Major Feature
        L18nStrings::WhatsNewReleaseNotesDnsModalHeader,   // Display name
        L18nStrings::WhatsNewReleaseNotesDnsListItemBody,  // Description
        L18nStrings::WhatsNewReleaseNotesDnsModalBodyTextNew,  // LongDescr
        "qrc:/ui/resources/features/custom-dns-preview.png",   // ImagePath
        "qrc:/ui/resources/settings/networkSettings.svg",      // IconPath
        "",                                                    // link URL
        "2.5",                                                 // released
        true,           // Can be flipped on
        false,          // Can be flipped off
        QStringList(),  // feature dependencies
        FeatureCallback_true)

FEATURE(
    easyProfile,     // Feature ID
    "Easy Profile",  // Feature name
    true,            // Is Major Feature
    L18nStrings::WhatsNewReleaseNotesEasyProfileDisplayName,  // Display name
    L18nStrings::Empty,                                       // Description
    L18nStrings::WhatsNewReleaseNotesEasyProfileDescription,  // LongDescr
    "qrc:/ui/resources/features/easy-profile-preview",        // ImagePath
    "",                                                       // IconPath
    "",                                                       // link URL
    "2.9",
    false,          // Can be flipped on
    false,          // Can be flipped off
    QStringList(),  // feature dependencies
    FeatureCallback_true)

FEATURE_SIMPLE(freeTrial,      // Feature ID
               "Free trial",   // Feature name
               "2.8.1",        // released
               true,           // Can be flipped on
               true,           // Can be flipped off
               QStringList(),  // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE_SIMPLE(inAppAccountCreate,                  // Feature ID
               "In-app Account Cretion",            // Feature name
               "2.6",                               // released
               true,                                // Can be flipped on
               false,                               // Can be flipped off
               QStringList{"inAppAuthentication"},  // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE_SIMPLE(inAppAuthentication,      // Feature ID
               "In-app Authentication",  // Feature name
               "2.4",                    // released
               true,                     // Can be flipped on
               false,                    // Can be flipped off
               QStringList(),            // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE_SIMPLE(inAppPurchase,      // Feature ID
               "In app Purchase",  // Feature name
               "2.4",              // released
               false,              // Can be flipped on
               false,              // Can be flipped off
               QStringList(),      // feature dependencies
               FeatureCallback_inAppPurchase)

FEATURE_SIMPLE(keyRegeneration,     // Feature ID
               "Key Regeneration",  // Feature name
               "2.10.0",            // released
               true,                // Can be flipped on
               true,                // Can be flipped off
               QStringList(),       // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(lanAccess,                    // Feature ID
               "Local area network access",  // Feature name
               "2.2",                        // released
               true,                         // Can be flipped on
               false,                        // Can be flipped off
               QStringList(),                // feature dependencies
               FeatureCallback_lanAccess)

FEATURE_SIMPLE(mobileOnboarding,     // Feature ID
               "Mobile Onboarding",  // Feature name
               "2.8",                // released
               true,                 // Can be flipped on
               false,                // Can be flipped off
               QStringList(),        // feature dependencies
               FeatureCallback_iosOrAndroid)

FEATURE(
    multiAccountContainers,      // Feature ID
    "Multi-Account Containers",  // Feature name
    true,                        // Is Major Feature
    L18nStrings::WhatsNewReleaseNotesMultiAccountContainersHeader,  // Display
                                                                    // name
    L18nStrings::
        WhatsNewReleaseNotesMultiAccountContainersShortDescription,  // Description
    L18nStrings::
        WhatsNewReleaseNotesMultiAccountContainersDescription,  // LongDescr
    "qrc:/ui/resources/features/"
    "multi-account-containers-preview.png",  // ImagePath
    "qrc:/ui/resources/features/multi-account-containers-icon.svg",  // IconPath
    "https://support.mozilla.org/kb/"
    "protect-your-container-tabs-mozilla-vpn?utm_source=mozilla.vpn."
    "client&utm_medium=in-app-link&utm_content=whats-new-panel&utm_"
    "campaign=vpn-better-together",  // link URL
    "2.7",
    false,          // Can be flipped on
    false,          // Can be flipped off
    QStringList(),  // feature dependencies
    FeatureCallback_multiAccountContainers)

#if defined(MVPN_ANDROID) || defined(MVPN_IOS)
#  define MULTIHOP_RELEASE "2.7"
#else
#  define MULTIHOP_RELEASE "2.5"
#endif

FEATURE(multiHop,                                            // Feature ID
        "Multi-hop",                                         // Feature name
        true,                                                // Is Major Feature
        L18nStrings::WhatsNewReleaseNotesMultiModalHeader,   // Display name
        L18nStrings::WhatsNewReleaseNotesMultiListItemBody,  // Description
        L18nStrings::WhatsNewReleaseNotesMultiModalBodyText,  // LongDescr
        "qrc:/ui/resources/features/multi-hop-preview.png",   // ImagePath
        "qrc:/nebula/resources/location-dark.svg",            // IconPath
        "",                                                   // link URL
        MULTIHOP_RELEASE,  // released for android
        true,              // Can be flipped on
        false,             // Can be flipped off
        QStringList(),     // feature dependencies
        FeatureCallback_true)

FEATURE_SIMPLE(notificationControl,     // Feature ID
               "Notification control",  // Feature name
               "2.3",                   // released
               true,                    // Can be flipped on
               false,                   // Can be flipped off
               QStringList(),           // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(serverUnavailableNotification,      // Feature ID
               "Server unavailable notification",  // Feature name
               "2.7",                              // released
               true,                               // Can be flipped on
               false,                              // Can be flipped off
               QStringList(),                      // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(shareLogs,      // Feature ID
               "Share Logs",   // Feature name
               "2.6",          // released
               false,          // Can be flipped on
               false,          // Can be flipped off
               QStringList(),  // feature dependencies
               FeatureCallback_shareLogs)

FEATURE_SIMPLE(splitTunnel,     // Feature ID
               "Split-tunnel",  // Feature name
               "2.4",           // released
               true,            // Can be flipped on
               false,           // Can be flipped off
               QStringList(),   // feature dependencies
               FeatureCallback_splitTunnel)

FEATURE_SIMPLE(startOnBoot,      // Feature ID
               "Start on boot",  // Feature name
               "2.0",            // released
               true,             // Can be flipped on
               false,            // Can be flipped off
               QStringList(),    // feature dependencies
               FeatureCallback_startOnBoot)

FEATURE_SIMPLE(subscriptionManagement,     // Feature ID
               "Subscription management",  // Feature name
               "2.9",                      // released
               true,                       // Can be flipped on
               true,                       // Can be flipped off
               QStringList(),              // feature dependencies
               FeatureCallback_true)

FEATURE_SIMPLE(tipsAndTricks,      // Feature ID
               "Tips and tricks",  // Feature name
               "2.9",              // released
               false,              // Can be flipped on
               true,               // Can be flipped off
               QStringList(),      // feature dependencies
               FeatureCallback_false)

FEATURE_SIMPLE(unsecuredNetworkNotification,      // Feature ID
               "Unsecured network notification",  // Feature name
               "2.2",                             // released
               true,                              // Can be flipped on
               false,                             // Can be flipped off
               QStringList(),                     // feature dependencies
               FeatureCallback_unsecuredNetworkNotification)

FEATURE_SIMPLE(websocket,      // Feature ID
               "WebSocket",    // Feature name
               "2.9.0",        // released
               true,           // Can be flipped on
               true,           // Can be flipped off
               QStringList(),  // feature dependencies
               FeatureCallback_true)
