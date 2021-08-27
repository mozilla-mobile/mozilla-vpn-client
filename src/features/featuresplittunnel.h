/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_SPLIT_TUNNEL_H
#define FEATURE_SPLIT_TUNNEL_H

#include "models/feature.h"
#include "l18nstrings.h"

#ifdef MVPN_LINUX
#  include <QProcessEnvironment>
#  include "platforms/linux/linuxdependencies.h"
#endif

constexpr const char* FEATURE_SPLIT_TUNNEL = "splitTunnel";

class FeatureSplitTunnel final : public Feature {
 public:
  FeatureSplitTunnel()
      : Feature(FEATURE_SPLIT_TUNNEL, L18nStrings::FeatureNamesSplittunnel,
                false,               // Is Major Feature
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.4",               // released
                true                 // Can be enabled in devmode
        ){};

  bool checkSupportCallback() const override {
#if defined(MVPN_ANDROID) || defined(MVPN_WINDOWS) || defined(MVPN_DUMMY)
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

  static const FeatureSplitTunnel* instance() {
    return static_cast<const FeatureSplitTunnel*>(get(FEATURE_SPLIT_TUNNEL));
  }
};

#endif  // FEATURE_SPLIT_TUNNEL_H
