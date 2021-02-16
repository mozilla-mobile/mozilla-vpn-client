/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featurelist.h"

#ifdef MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

namespace {
FeatureList s_featureList;
}

// static
FeatureList* FeatureList::instance() { return &s_featureList; }

bool FeatureList::startOnBootSupported() const {
#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS)
  return true;
#elif defined(MVPN_ANDROID)
  return AndroidUtils::canEnableStartOnBoot();
#else
  return false;
#endif
}

bool FeatureList::localNetworkAccessSupported() const {
#if defined(MVPN_IOS)
  // managed by the OS automatically. No need to expose this feature.
  return false;
#endif

  // All the rest (android, windows, linux, mac,...) is OK.
  return true;
}

bool FeatureList::protectSelectedAppsSupported() const {
#if defined(MVPN_ANDROID)
  return true;
#else
  return false;
#endif
}

bool FeatureList::captivePortalNotificationSupported() const {
#if defined(MVPN_WINDOWS) || defined(MVPN_WASM)
  return true;
#else
  return false;
#endif
}

bool FeatureList::unsecuredNetworkNotificationSupported() const {
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS) || \
    defined(MVPN_WASM)
  return true;
#else
  return false;
#endif
}
