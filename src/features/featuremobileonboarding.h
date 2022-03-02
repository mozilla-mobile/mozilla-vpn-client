/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_MOBILE_ONBOARDING_H
#define FEATURE_MOBILE_ONBOARDING_H

#include "models/feature.h"

constexpr const char* FEATURE_MOBILE_ONBOARDING = "mobileOnboarding";

class FeatureMobileOnboarding final : public Feature {
 public:
  FeatureMobileOnboarding()
      : Feature(FEATURE_MOBILE_ONBOARDING, "Mobile Onboarding",
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

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
    return true;
#else
    return false;
#endif
  }

  static const FeatureMobileOnboarding* instance() {
    return static_cast<const FeatureMobileOnboarding*>(
        get(FEATURE_MOBILE_ONBOARDING));
  }
};

#endif  // FEATURE_MOBILE_ONBOARDING_H
