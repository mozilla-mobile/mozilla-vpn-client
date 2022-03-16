/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_INCLUDEALLNETWORKS_H
#define FEATURE_INCLUDEALLNETWORKS_H

#include "models/feature.h"

constexpr const char* FEATURE_INCLUDEALLNETWORKS = "includeAllNetworks";

class FeatureIncludeAllNetworks : public Feature {
 public:
  FeatureIncludeAllNetworks()
      : Feature(FEATURE_INCLUDEALLNETWORKS, "Include All Networks",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.8",               // released
                true                // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) 
    return true;
#else
    return false;
#endif
  }

  static const FeatureIncludeAllNetworks* instance() {
    return static_cast<const FeatureIncludeAllNetworks*>(get(FEATURE_INCLUDEALLNETWORKS));
  }
};

#endif  // FEATURE_INCLUDEALLNETWORKS_H
