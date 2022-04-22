/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_IN_APP_ACCOUNT_CREATE_H
#define FEATURE_IN_APP_ACCOUNT_CREATE_H

#include "models/feature.h"

constexpr const char* FEATURE_IN_APP_ACCOUNT_CREATE = "inAppAccountCreate";

class FeatureInAppAccountCreate final : public Feature {
 public:
  FeatureInAppAccountCreate()
      : Feature(FEATURE_IN_APP_ACCOUNT_CREATE, "In app account creation",
                false,                              // Is Major Feature
                L18nStrings::Empty,                 // Display name
                L18nStrings::Empty,                 // Description
                L18nStrings::Empty,                 // LongDescr
                "",                                 // ImagePath
                "",                                 // IconPath
                "",                                 // link URL
                "2.6",                              // released
                true,                               // Can be enabled in devmode
                QStringList{"inAppAuthentication"}  // feature dependencies
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
    return true;
#else
    return false;
#endif
  }

  static const FeatureInAppAccountCreate* instance() {
    return static_cast<const FeatureInAppAccountCreate*>(
        get(FEATURE_IN_APP_ACCOUNT_CREATE));
  }
};

#endif  // FEATURE_IN_APP_ACCOUNT_CREATE_H
