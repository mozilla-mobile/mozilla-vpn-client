/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURES_H
#define FEATURES_H

#include "featuretypes.h"

#include <QtGlobal>

#include "constants.h"

namespace Platform {

#ifdef MZ_WINDOWS
inline constexpr bool windows = true;
#else
inline constexpr bool windows = false;
#endif

#ifdef MZ_MACOS
inline constexpr bool macos = true;
#else
inline constexpr bool macos = false;
#endif

#ifdef MZ_LINUX
inline constexpr bool linux_ = true;
#else
inline constexpr bool linux_ = false;
#endif

#ifdef MZ_ANDROID
inline constexpr bool android = true;
#else
inline constexpr bool android = false;
#endif

enum class AndroidFlavor { foss, gps };

constexpr bool isAndroidFlavor(AndroidFlavor flavor) {
#if defined(MZ_ANDROID) && defined(MZ_ANDROID_FOSS_BUILD)
  return flavor == AndroidFlavor::foss;
#elif defined(MZ_ANDROID)
  return flavor == AndroidFlavor::gps;
#else
  Q_UNUSED(flavor);
  return false;
#endif
}

#ifdef MZ_IOS
inline constexpr bool ios = true;
#else
inline constexpr bool ios = false;
#endif

#ifdef MZ_WASM
inline constexpr bool wasm = true;
#else
inline constexpr bool wasm = false;
#endif

#ifdef MVPN_BALROG
inline constexpr bool hasBalrog = true;
#else
inline constexpr bool hasBalrog = false;
#endif

#ifdef UNIT_TEST
inline constexpr bool unitTest = true;
#else
inline constexpr bool unitTest = false;
#endif

constexpr bool qtAtLeast(int major, int minor, int patch) {
  return QT_VERSION_MAJOR > major ||
         (QT_VERSION_MAJOR == major &&
          (QT_VERSION_MINOR > minor ||
           (QT_VERSION_MINOR == minor && QT_VERSION_PATCH >= patch)));
}

}  // namespace Platform

namespace Feature {

// ============================================================================
// How to add a feature
// ============================================================================
//
// 1. Pick the right tier:
//
//    ConstantFeature   - Value known at compile time. Zero runtime cost.
//                        Use when the flag depends only on platform, build
//                        defines, or Qt version.
//                        Example: .supported = Platform::android || Platform::ios
//
//    RuntimeFeature    - Value determined at runtime but cannot be overridden.
//                        Use when you need to check something that isn't known
//                        at compile time (e.g. OS version, hardware caps).
//                        Define in features.cpp if it needs platform headers.
//                        Example: Android SDK version check for shareLogs.
//
//    OverridableFeature - Value can be overridden by the dev menu or the
//                        Guardian API (/api/v1/features). Use when QA or
//                        the server needs to flip the feature at runtime.
//                        Provide an evaluator lambda that returns the default.
//                        Example: .evaluator = +[] { return false; }
//
// 2. Declare the feature below in the appropriate tier section.
//    For features that need platform-specific includes in their evaluator,
//    declare as `extern const` here and define in features.cpp.
//
// 3. Model exposure:
//    If QML needs to read the feature (e.g. to show/hide UI), or Guardian /
//    the dev menu needs to flip it, add it to s_exposedFeatures in
//    featuremodel.h. QML accesses it via:
//      MZFeatureList.get("featureId").isSupported
//
// 4. C++ usage:
//    #include "feature/features.h"
//    if (Feature::myFeature.supported) { ... }       // ConstantFeature
//    if (Feature::isEnabled(Feature::myFeature)) { ... } // any tier
//
// 5. Tests / functional tests:
//    If functional tests need to flip the feature (flipFeatureOn/Off), it
//    must be an OverridableFeature and listed in s_exposedFeatures.
//    The inspector commands flip_on_feature / flip_off_feature only work
//    on OverridableFeatures that are in that list.
//
// ============================================================================

// ---- Tier 1: ConstantFeature (constexpr, zero-cost) ----------------------


inline constexpr ConstantFeature appReview = {
    .id = "appReview",
    .name = "App Review",
    .supported = Platform::android || Platform::ios,
};

inline constexpr ConstantFeature customDNS = {
    .id = "customDNS",
    .name = "Custom DNS",
    .supported = true,
};

inline const OverridableFeature enableUpdateServer = {
    .id = "enableUpdateServer",
    .name = "Enable Update Server",
    .evaluator = +[] { return Platform::hasBalrog; },
    .canFlipOn = +[] { return Platform::hasBalrog; },
    .canFlipOff = +[] { return Platform::hasBalrog; },
};


inline constexpr ConstantFeature gleanRust = {
    .id = "gleanRust",
    .name = "Glean Rust SDK",
    .supported = true,
};

inline constexpr ConstantFeature keyRegeneration = {
    .id = "keyRegeneration",
    .name = "Key Regeneration",
    .supported = true,
};

inline constexpr ConstantFeature multiHop = {
    .id = "multiHop",
    .name = "Multi-hop",
    .supported = true,
};

inline constexpr ConstantFeature notificationControl = {
    .id = "notificationControl",
    .name = "Notification control",
    .supported = true,
};

inline constexpr ConstantFeature sentry = {
    .id = "sentry",
    .name = "Sentry Crash Report SDK",
    .supported = true,
};

inline constexpr ConstantFeature serverConnectionScore = {
    .id = "serverConnectionScore",
    .name = "Server connection score",
    .supported = true,
};

inline constexpr ConstantFeature serverUnavailableNotification = {
    .id = "serverUnavailableNotification",
    .name = "Server unavailable notification",
    .supported = true,
};

inline constexpr ConstantFeature stagingUpdateServer = {
    .id = "stagingUpdateServer",
    .name = "Staging Update Server",
    .supported = false,
};


inline constexpr ConstantFeature superDooperMetrics = {
    .id = "superDooperMetrics",
    .name = "Super Dooper Metrics",
    .supported = true,
};

inline constexpr ConstantFeature webPurchase = {
    .id = "webPurchase",
    .name = "Web Purchase",
    .supported = Platform::windows || Platform::macos || Platform::linux_ ||
                 Platform::isAndroidFlavor(Platform::AndroidFlavor::foss),
};

#if defined(MZ_ANDROID)
extern const RuntimeFeature shareLogs;
#else
inline constexpr ConstantFeature shareLogs = {
    .id = "shareLogs",
    .name = "Share Logs",
    .supported = Platform::windows || Platform::linux_ || Platform::macos ||
                 Platform::ios || Platform::wasm,
};
#endif

inline constexpr ConstantFeature themeSelectionIncludesAutomatic = {
    .id = "themeSelectionIncludesAutomatic",
    .name = "Theme option of 'automatic'",
    .supported = Platform::linux_ || Platform::qtAtLeast(6, 5, 0),
};

inline const OverridableFeature accountDeletion = {
    .id = "accountDeletion",
    .name = "Account deletion",
    .evaluator = +[] { return Platform::android || Platform::ios; },
};

inline const OverridableFeature alwaysPort53 = {
    .id = "alwaysPort53",
    .name = "Always use port 53",
    .evaluator = +[] { return false; },
};

inline constexpr ConstantFeature annualUpgrade = {
    .id = "annualUpgrade",
    .name = "Annual upgrade",
    .evaluator = +[] {
      return Platform::windows || Platform::macos || Platform::linux_ ||
             Platform::wasm;
    },
};

inline constexpr ConstantFeature captivePortal = {
    .id = "captivePortal",
    .name = "Captive Portal",
    .evaluator = +[] {
      return Platform::linux_ || Platform::macos || Platform::windows ||
             Platform::wasm;
    },
    .canFlipOn = +[] { return true; },
    .canFlipOff = +[] { return false; },
};

inline constexpr ConstantFeature checkConnectivityOnActivation = {
    .id = "checkConnectivityOnActivation",
    .name = "Check Connectivity On Activation",
    .supported = false,
};

inline const OverridableFeature factoryReset = {
    .id = "factoryReset",
    .name = "Factory reset",
    .evaluator = +[] { return true; },
};

inline constexpr ConstantFeature freeTrial = {
    .id = "freeTrial",
    .name = "Free trial",
    .supported = false,
};

inline const OverridableFeature replacerAddon = {
    .id = "replacerAddon",
    .name = "Replacer Addon",
    .evaluator = +[] { return false; },
};

inline const OverridableFeature recommendedServers = {
    .id = "recommendedServers",
    .name = "Recommended servers",
    .evaluator = +[] { return true; },
};

inline const OverridableFeature subscriptionManagement = {
    .id = "subscriptionManagement",
    .name = "Subscription management",
    .evaluator = +[] { return true; },
};

inline constexpr ConstantFeature startOnBoot = {
    .id = "startOnBoot",
    .name = "Start on boot",
    .evaluator = +[] {
      return Platform::windows || Platform::macos || Platform::linux_;
    },
};

inline constexpr ConstantFeature unsecuredNetworkNotification = {
    .id = "unsecuredNetworkNotification",
    .name = "Unsecured network notification",
    .evaluator = +[] {
      return Platform::windows || Platform::macos || Platform::linux_;
    },
};

inline const OverridableFeature addonSignature = {
    .id = "addonSignature",
    .name = "Addons Signature",
    .evaluator = +[] { return true; },
    .canFlipOn = +[] { return !Constants::inProduction(); },
    .canFlipOff = +[] { return !Constants::inProduction(); },
};

inline const OverridableFeature inAppAuthentication = {
    .id = "inAppAuthentication",
    .name = "In-app Authentication",
    .evaluator = +[]() -> bool { return Platform::wasm; },
};

inline const OverridableFeature inAppAccountCreate = {
    .id = "inAppAccountCreate",
    .name = "In-app Account Creation",
    .evaluator = +[]() -> bool { return isEnabled(inAppAuthentication); },
};

inline const OverridableFeature webExtension = {
    .id = "webExtension",
    .name = "webExtension",
    .evaluator = +[] { return Platform::windows; },
    .canFlipOn = +[]() -> bool {
      return Platform::windows || Platform::macos || Platform::linux_;
    },
};

extern const OverridableFeature splitTunnel;

}  // namespace Feature

#endif  // FEATURES_H
