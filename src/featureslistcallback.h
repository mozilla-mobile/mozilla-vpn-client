/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURELISTCALLBACK_H
#define FEATURELISTCALLBACK_H

#ifdef MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

// Generic callback functions
// --------------------------

bool FeatureCallback_true() { return true; }

bool FeatureCallback_false() { return false; }

bool FeatureCallback_iosOrAndroid() {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
  return true;
#else
  return false;
#endif
}

bool FeatureCallback_inStaging() { return !Constants::inProduction(); }

// Custom callback functions
// -------------------------

bool FeatureCallback_accountDeletion() {
#if defined(MVPN_IOS)
  return true;
#else
  return false;
#endif
}

bool FeatureCallback_webPurchase() {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID) || defined(MVPN_WASM)
  return false;
#else
  return true;
#endif
}

bool FeatureCallback_shareLogs() {
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS) || \
    defined(MVPN_IOS) || defined(MVPN_DUMMY)
  return true;
#elif defined(MVPN_ANDROID)
  return AndroidUtils::GetSDKVersion() >=
         29;  // Android Q (10) is required for this
#else
  return false;
#endif
}

bool FeatureCallback_startOnBoot() {
#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS) || \
    defined(MVPN_DUMMY) || defined(MVPN_WASM) || defined(MVPN_ANDROID)
  return true;
#else
  return false;
#endif
}

bool FeatureCallback_freeTrial() {
#if defined(MVPN_IOS)
  return true;
#else
  return false;
#endif
}

#include "productfeatureslistcallback.h"

#endif  // FEATURELISTCALLBACK_H
