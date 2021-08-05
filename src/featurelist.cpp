/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featurelist.h"
#include "controller.h"
#include "mozillavpn.h"

#include <QProcessEnvironment>

#ifdef MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

namespace {
FeatureList s_featureList;
}

// static
FeatureList* FeatureList::instance() { return &s_featureList; }

bool FeatureList::startOnBootSupported() const {
#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS) || \
    defined(MVPN_DUMMY) || defined(MVPN_WASM)
  return true;
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
  auto controller = MozillaVPN::instance()->controller();
  return controller->getCapabilities().isSplitTunnelSupported();

}

bool FeatureList::captivePortalNotificationSupported() const {
#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS) || \
    defined(MVPN_DUMMY) || defined(MVPN_WASM)
  return true;
#else
  // If we decide to enable the captive-portal notification for IOS, remember
  // to add the following keys/values in the ios/app/Info.plist:
  // ```
  // <key>NSAppTransportSecurity</key>
  // <dict>
  //   <key>NSAllowsArbitraryLoads</key>
  //   <true/>
  // </dict>
  // ```
  // NSAllowsArbitraryLoads allows the loading of HTTP (not-encrypted)
  // requests. By default, IOS apps work in HTTPS-only mode.
  return false;
#endif
}

bool FeatureList::unsecuredNetworkNotificationSupported() const {
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS) || \
    defined(MVPN_WASM) || defined(MVPN_DUMMY)
  return true;
#else
  return false;
#endif
}

bool FeatureList::notificationControlSupported() const {
#if defined(MVPN_ANDROID)
  return false;
#else
  return true;
#endif
}

bool FeatureList::userDNSSupported() const {
#if defined(MVPN_ANDROID) || defined(MVPN_WASM) || defined(MVPN_DUMMY) || \
    defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
  return true;
#else
  return false;
#endif
}

bool FeatureList::authenticationInApp() const { return false; }
