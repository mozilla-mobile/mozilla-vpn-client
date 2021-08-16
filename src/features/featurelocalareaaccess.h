/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_LAN_ACCESS_H
#define FEATURE_LAN_ACCESS_H

#include "models/feature.h"
#include "l18nstrings.h"

constexpr const char* FEATURE_LAN_ACCESS = "lanAccess";

class FeatureLocalAreaAccess final : public Feature {
 public:
  FeatureLocalAreaAccess()
      : Feature(FEATURE_LAN_ACCESS, L18nStrings::FeatureNamesLocalAreaAccess,
                false,               // Is Major Feature
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.2",               // released
                true                 // Can be enabled in devmode
        ){};

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS)
    // managed by the OS automatically. No need to expose this feature.
    return false;
#endif

    // All the rest (android, windows, linux, mac,...) is OK.
    return true;
  }

  static const FeatureLocalAreaAccess* instance() {
    return static_cast<const FeatureLocalAreaAccess*>(get(FEATURE_LAN_ACCESS));
  }
};

#endif  // FEATURE_LAN_ACCESS_H
