/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_STARTONBOOT_H
#define FEATURE_STARTONBOOT_H

#include "models/feature.h"

constexpr const char* FEATURE_STARTONBOOT = "startOnBoot";

class FeatureStartOnBoot final : public Feature {
 public:
  FeatureStartOnBoot()
      : Feature(FEATURE_STARTONBOOT, "Start on boot",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.0",               // released
                true                 // Can be enabled in devmode
        ){};

  bool checkSupportCallback() const override {
#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS) || \
    defined(MVPN_DUMMY) || defined(MVPN_WASM)
    return true;
#else
    return false;
#endif
  }

  static const FeatureStartOnBoot* instance() {
    return static_cast<const FeatureStartOnBoot*>(get(FEATURE_STARTONBOOT));
  }
};

#endif  // FEATURE_STARTONBOOT_H
