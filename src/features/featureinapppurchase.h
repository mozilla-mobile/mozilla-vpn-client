/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_IN_APP_PURCHASE_H
#define FEATURE_IN_APP_PURCHASE_H

#include "models/feature.h"

constexpr const char* FEATURE_IN_APP_PURCHASE = "inAppPurchase";

class FeatureInAppPurchase final : public Feature {
 public:
  FeatureInAppPurchase()
      : Feature(FEATURE_IN_APP_PURCHASE, "In app Purchase",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.4",               // released
                false                // Can be enabled in devmode
        ){};

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
    return true;
#else
    return false;
#endif
  }

  static const FeatureInAppPurchase* instance() {
    return static_cast<const FeatureInAppPurchase*>(
        get(FEATURE_IN_APP_PURCHASE));
  }
};

#endif  // FEATURE_IN_APP_PURCHASE_H
