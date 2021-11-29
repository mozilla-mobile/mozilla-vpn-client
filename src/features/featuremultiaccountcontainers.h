/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_MULTI_ACCOUNT_CONTAINERS_H
#define FEATURE_MULTI_ACCOUNT_CONTAINERS_H

#include "models/feature.h"

constexpr const char* FEATURE_MULTI_ACCOUNT_CONTAINERS =
    "multiAccountContainers";

class FeatureMultiAccountContainers final : public Feature {
 public:
  FeatureMultiAccountContainers()
      : Feature(FEATURE_MULTI_ACCOUNT_CONTAINERS, "Multi-Account Containers",
                true,                // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.7",               // released
                false                // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
    // Multi-Account Containers is a Firefox add-on only released for desktop
    return false;
#endif

    return true;
  }

  static const FeatureMultiAccountContainers* instance() {
    return static_cast<const FeatureMultiAccountContainers*>(
        get(FEATURE_MULTI_ACCOUNT_CONTAINERS));
  }
};

#endif  // FEATURE_MULTI_ACCOUNT_CONTAINERS_H
