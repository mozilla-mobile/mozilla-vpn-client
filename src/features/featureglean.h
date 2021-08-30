/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_GLEAN_H
#define FEATURE_GLEAN_H

#include "models/feature.h"

constexpr const char* FEATURE_GLEAN = "glean";

class FeatureGlean : public Feature {
 public:
  FeatureGlean()
      : Feature(FEATURE_GLEAN, "Glean",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.5",               // released
                false                // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS)
    // https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1599
    return false;
#else
    return true;
#endif
  }

  static const FeatureGlean* instance() {
    return static_cast<const FeatureGlean*>(get(FEATURE_GLEAN));
  }
};

#endif  // FEATURE_GLEAN_H
