/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_SUBSCRIPTION_MANAGEMENT_H
#define FEATURE_SUBSCRIPTION_MANAGEMENT_H

#include "models/feature.h"

constexpr const char* FEATURE_SUBSCRIPTION_MANAGEMENT = "subscriptionManagement";

class FeatureSubscriptionManagement final : public Feature {
 public:
  FeatureSubscriptionManagement()
      : Feature(FEATURE_SUBSCRIPTION_MANAGEMENT, "Subscription management",
                false,                              // Is Major Feature
                L18nStrings::Empty,                 // Display name
                L18nStrings::Empty,                 // Description
                L18nStrings::Empty,                 // LongDescr
                "",                                 // ImagePath
                "",                                 // IconPath
                "",                                 // link URL
                "2.9",                              // released
                true,                               // Can be flipped on
                false,                              // Can be flipped off
                QStringList()                       // feature dependencies
        ) {}

  bool checkSupportCallback() const override {
    return false;
  }

  static const FeatureSubscriptionManagement* instance() {
    return static_cast<const FeatureSubscriptionManagement*>(
        get(FEATURE_SUBSCRIPTION_MANAGEMENT));
  }
};

#endif  // FEATURE_SUBSCRIPTION_MANAGEMENT_H
