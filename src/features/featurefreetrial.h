/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_FREETRIAL_H
#define FEATURE_FREETRIAL_H

#include "models/feature.h"

constexpr const char* FEATURE_FREETRIAL = "freeTrial";

class FeatureFreeTrial : public Feature {
 public:
  FeatureFreeTrial()
      : Feature(FEATURE_FREETRIAL, "Free trial",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.8.1",             // released
                true,                // Can be flipped on
                true,                // Can be flipped off
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
    return true;
#else
    return false;
#endif
  }

  static const FeatureFreeTrial* instance() {
    return static_cast<const FeatureFreeTrial*>(get(FEATURE_FREETRIAL));
  }
};

#endif  // FEATURE_FREETRIAL_H
