/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURELISTCALLBACK_H
#define FEATURELISTCALLBACK_H

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

// Generic callback functions
// --------------------------

bool FeatureCallback_true() { return true; }

bool FeatureCallback_false() { return false; }

bool FeatureCallback_inStaging() { return !Constants::inProduction(); }

// Custom callback functions
// -------------------------

bool FeatureCallback_shareLogs() {
#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS) || \
    defined(MZ_IOS) || defined(MZ_DUMMY)
  return true;
#elif defined(MZ_ANDROID)
  return AndroidCommons::getSDKVersion() >=
         29;  // Android Q (10) is required for this
#else
  return false;
#endif
}

// The app must implement its feature list callback file.
#include "appfeaturelistcallback.h"

#endif  // FEATURELISTCALLBACK_H
