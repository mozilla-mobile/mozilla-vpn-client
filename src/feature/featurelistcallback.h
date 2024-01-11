/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURELISTCALLBACK_H
#define FEATURELISTCALLBACK_H

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

#ifdef MZ_WINDOWS
#  include "platforms/windows/daemon/windowssplittunnel.h"
#endif

#ifdef MZ_LINUX
#  include <QProcessEnvironment>

#  include "platforms/linux/linuxdependencies.h"
#  include "versionutils.h"
#endif

// Generic callback functions
// --------------------------

bool FeatureCallback_true() { return true; }

bool FeatureCallback_false() { return false; }

bool FeatureCallback_inStaging() { return !Constants::inProduction(); }

bool FeatureCallback_iosOrAndroid() {
#if defined(MZ_IOS) || defined(MZ_ANDROID)
  return true;
#else
  return false;
#endif
}

// Custom callback functions
// -------------------------

bool FeatureCallback_annualUpgrade() {
  if (FeatureCallback_iosOrAndroid()) {
    return false;
  }
  return true;
}

bool FeatureCallback_sentry() {
#if defined(MZ_IOS)
  return FeatureCallback_inStaging();
#else
  return true;
#endif
}

bool FeatureCallback_captivePortal() {
#if defined(MZ_LINUX) || defined(MZ_MACOS) || defined(MZ_WINDOWS) || \
    defined(MZ_DUMMY) || defined(MZ_WASM)
  return true;
#else
  // If we decide to enable the captive-portal notification for
  // IOS, remember to add the following keys/values in the
  // ios/app/Info.plist:
  // ```
  // <key>NSAppTransportSecurity</key>
  // <dict>
  //   <key>NSAllowsArbitraryLoads</key>
  //   <true/>
  // </dict>
  // ```
  // NSAllowsArbitraryLoads allows the loading of HTTP
  // (not-encrypted) requests. By default, IOS apps work in
  // HTTPS-only mode.
  return false;
#endif
}

bool FeatureCallback_splitTunnel() {
#if defined(MZ_ANDROID) || defined(MZ_DUMMY)
  return true;
#elif defined(MZ_WINDOWS)
  return !WindowsSplitTunnel::detectConflict();
#elif defined(MZ_LINUX)
  static bool initDone = false;
  static bool splitTunnelSupported = false;
  if (initDone) {
    return splitTunnelSupported;
  }
  initDone = true;

  /* Control groups v2 must be mounted for app/traffic classification
   */
  if (LinuxDependencies::findCgroup2Path().isNull()) {
    return false;
  }

  /* Application tracking is only supported on GTK-based desktops
   */
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (!pe.contains("XDG_CURRENT_DESKTOP")) {
    return false;
  }
  QStringList desktop = pe.value("XDG_CURRENT_DESKTOP").split(":");
  if (desktop.contains("GNOME")) {
    QString shellVersion = LinuxDependencies::gnomeShellVersion();
    if (shellVersion.isNull()) {
      return false;
    }
    if (VersionUtils::compareVersions(shellVersion, "3.34") < 0) {
      return false;
    }
  } else if (desktop.contains("KDE")) {
    QString kdeVersion = LinuxDependencies::kdeFrameworkVersion();
    if (kdeVersion.isNull()) {
      return false;
    }
    /* The metadata we need (SourcePath) is only added since kio v5.75
     */
    if (VersionUtils::compareVersions(kdeVersion, "5.75") < 0) {
      return false;
    }
  }
  // TODO: These shells need more testing.
  else if (!desktop.contains("MATE") && !desktop.contains("Unity") &&
           !desktop.contains("X-Cinnamon")) {
    return false;
  }
  splitTunnelSupported = true;
  return splitTunnelSupported;
#else
  return false;
#endif
}

bool FeatureCallback_startOnBoot() {
#if defined(MZ_LINUX) || defined(MZ_MACOS) || defined(MZ_WINDOWS) || \
    defined(MZ_DUMMY) || defined(MZ_WASM)
  return true;
#else
  return false;
#endif
}

bool FeatureCallback_unsecuredNetworkNotification() {
#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS) || \
    defined(MZ_WASM) || defined(MZ_DUMMY)
  return true;
#else
  return false;
#endif
}

// Free trials are currently not being used on any platforms
// Leaving this code in case we want to re-enable them in the future
bool FeatureCallback_freeTrial() {
#if defined(MZ_IOS)
  return true;
#else
  return false;
#endif
}

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

bool FeatureCallback_swipeToGoBack() {
  return false; //while the feature is still unreleased
#if defined(MZ_IOS)
  return true;
#else
  return false;
#endif
}


bool FeatureCallback_webPurchase() {
#if defined(MZ_IOS) || defined(MZ_ANDROID) || defined(MZ_WASM)
  return false;
#else
  return true;
#endif
}

#endif  // FEATURELISTCALLBACK_H
