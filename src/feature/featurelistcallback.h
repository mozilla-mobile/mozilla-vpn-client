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

struct FeatureSupportedPlatforms {
  bool windows = false;
  bool macos = false;
  bool gnu_linux = false;  // TIL, "linux" is reserved keyword on gcc.
  bool android = false;
  bool ios = false;
  bool wasm = false;
};

// TODO : Remove once focal support is removed
// as that is that is the only distro we support
// that does not have consteval q_q
#if defined(__cpp_consteval)  // If consteval is supported
#  define constIshExpr consteval
#else  // Fallback to constexpr
#  define constIshExpr constexpr
#endif

constIshExpr auto enableForPlatform(FeatureSupportedPlatforms support) {
  return [support]() constexpr {
#if defined(MZ_WINDOWS)
    return support.windows;
#elif defined(MZ_MACOS)
    return support.macos;
#elif defined(MZ_LINUX)
    return support.gnu_linux;
#elif defined(MZ_ANDROID)
    return support.android;
#elif defined(MZ_IOS)
    return support.ios;
#elif defined(MZ_WASM)
    return support.wasm;
#else
    return false;
#endif
  };
}
#undef constIshExpr

// Custom callback functions
// -------------------------

bool FeatureCallback_captivePortal() {
#if defined(MZ_LINUX) || defined(MZ_MACOS) || defined(MZ_WINDOWS) || \
    defined(MZ_WASM)
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

bool FeatureCallback_inAppAuthentication() {
#if defined(MZ_WASM)
  return true;
#else
  if (Constants::inProduction() || enableForPlatform({
                                       .android = true,
                                       .ios = true,
                                   })()) {
    return true;
  }

  return false;
#endif
}

bool FeatureCallback_splitTunnel() {
#if defined(MZ_ANDROID) || defined(MZ_WASM) || defined(UNIT_TEST)
  return true;
#elif defined(MZ_WINDOWS)
  return !WindowsSplitTunnel::detectConflict();
#elif defined(MZ_LINUX) && !defined(MZ_FLATPAK)
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

bool FeatureCallback_shareLogs() {
#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS) || \
    defined(MZ_IOS) || defined(MZ_WASM)
  return true;
#elif defined(MZ_ANDROID)
  return AndroidCommons::getSDKVersion() >=
         29;  // Android Q (10) is required for this
#else
  return false;
#endif
}

bool FeatureCallback_hasBalrog() {
#if defined(MVPN_BALROG)
  return true;
#else
  return false;
#endif
}

bool FeatureCallback_proxyCanTurnOn() {
#if defined(MZ_WINDOWS) || defined(MZ_LINUX)
  return true;
#else
  return false;
#endif
}

#endif  // FEATURELISTCALLBACK_H
