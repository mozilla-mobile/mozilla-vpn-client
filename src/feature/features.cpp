/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "features.h"

#include <QVersionNumber>

#if defined(MZ_WINDOWS) && !defined(UNIT_TEST)
#  include "platforms/windows/daemon/windowssplittunnel.h"
#endif

#if defined(MZ_LINUX) && !defined(UNIT_TEST)
#  include <QProcessEnvironment>

#  include "platforms/linux/linuxutils.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

namespace Feature {

#if defined(MZ_WINDOWS) && !defined(UNIT_TEST)
const OverridableFeature splitTunnel = {
    .id = "splitTunnel",
    .name = "Split-tunnel",
    .evaluator = +[] { return !WindowsSplitTunnel::detectConflict(); },
};
#elif defined(MZ_LINUX) && !defined(MZ_FLATPAK) && !defined(UNIT_TEST)
const OverridableFeature splitTunnel = {
    .id = "splitTunnel",
    .name = "Split-tunnel",
    .evaluator = +[]() -> bool {
      static bool initDone = false;
      static bool supported = false;
      if (initDone) return supported;
      initDone = true;

      if (LinuxUtils::findCgroup2Path().isNull()) return false;

      QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
      if (!pe.contains("XDG_CURRENT_DESKTOP")) return false;

      QStringList desktop = pe.value("XDG_CURRENT_DESKTOP").split(":");
      if (desktop.contains("GNOME")) {
        auto v = LinuxUtils::gnomeShellVersion();
        if (v.isNull() || v < QVersionNumber(3, 34)) return false;
      } else if (desktop.contains("KDE")) {
        auto v = LinuxUtils::kdePlasmaVersion();
        if (v.isNull() || v < QVersionNumber(5, 24)) return false;
      } else {
        return false;
      }
      supported = true;
      return true;
    },
};
#else
const OverridableFeature splitTunnel = {
    .id = "splitTunnel",
    .name = "Split-tunnel",
    .evaluator =
        +[] {
          return Platform::android || Platform::wasm || Platform::unitTest;
        },
};
#endif

// --- Android features ---

#ifdef MZ_ANDROID
const RuntimeFeature shareLogs = {
    .id = "shareLogs",
    .name = "Share Logs",
    .evaluator = +[] { return AndroidCommons::getSDKVersion() >= 29; },
};
#endif

}  // namespace Feature
