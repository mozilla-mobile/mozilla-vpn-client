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

FEATURE_SIMPLE(qmlpath,                // Feature ID
               "QMLPath support",      // Feature name
               "2.13.0",               // released
               FeatureCallback_false,  // Can be flipped on
               FeatureCallback_false,  // Can be flipped off
               QStringList(),          // feature dependencies
               FeatureCallback_true)

// The app must implement its feature list file.
#include "appfeaturelist.h"
