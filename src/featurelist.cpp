/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featurelist.h"

#include <QProcessEnvironment>

#ifdef MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

#ifdef MVPN_LINUX
#  include "platforms/linux/linuxdependencies.h"
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
#if defined(MVPN_ANDROID) || defined(MVPN_WINDOWS)
  return true;
#elif defined(MVPN_LINUX)
  static bool initDone = false;
  static bool splitTunnelSupported = false;
  if (initDone) {
    return splitTunnelSupported;
  }
  initDone = true;

  /* Control groups v1 must be mounted for traffic classification */
  if (LinuxDependencies::findCgroupPath("net_cls").isNull()) {
    return false;
  }

  /* Application tracking is only supported on GTK-based desktops */
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (!pe.contains("XDG_CURRENT_DESKTOP")) {
    return false;
  }
  QStringList desktop = pe.value("XDG_CURRENT_DESKTOP").split(":");
  if (!desktop.contains("GNOME") && !desktop.contains("MATE") &&
      !desktop.contains("Unity") && !desktop.contains("X-Cinnamon")) {
    return false;
  }
  splitTunnelSupported = true;
  return splitTunnelSupported;
#else
  return false;
#endif
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
  // Disabled for 2.4
  return false;
}