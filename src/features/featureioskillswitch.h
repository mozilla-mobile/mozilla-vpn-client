/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MVPN_IOS
#error "This feature should be exposed only to IOS"
#endif

#ifndef FEATURE_IOS_KILLSWITCH_H
#define FEATURE_IOS_KILLSWITCH_H

#include "models/feature.h"

constexpr const char* FEATURE_IOS_KILLSWITCH = "iosKillswitch";

class FeatureIosKillswitch : public Feature {
 public:
  FeatureIosKillswitch()
      : Feature(FEATURE_IOS_KILLSWITCH, "iOS Kill Switch",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.8",               // released
                true                 // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override { return true; }

  static const FeatureIosKillswitch* instance() {
    return static_cast<const FeatureIosKillswitch*>(
        get(FEATURE_IOS_KILLSWITCH));
  }
};

#endif  // FEATURE_IOS_KILLSWITCH_H
