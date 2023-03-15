/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE! Do not include this file directly. Use featurelistcallback.h instead.

#ifdef MZ_WINDOWS
#  include "platforms/windows/daemon/windowssplittunnel.h"
#endif

#ifdef MZ_LINUX
#  include <QProcessEnvironment>

#  include "platforms/linux/linuxdependencies.h"
#  include "versionutils.h"
#endif

// Custom callback functions
// -------------------------

bool FeatureCallback_accountDeletion() {
#if defined(MZ_IOS)
  return true;
#else
  return false;
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
    defined(MZ_DUMMY) || defined(MZ_WASM) || defined(MZ_ANDROID)
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

bool FeatureCallback_freeTrial() {
#if defined(MZ_IOS)
  return true;
#else
  return false;
#endif
}
