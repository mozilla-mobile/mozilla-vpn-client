/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_APPREVIEW_H
#define FEATURE_APPREVIEW_H

#include "models/feature.h"

constexpr const char* FEATURE_APPREVIEW = "appReview";

class FeatureAppReview : public Feature {
 public:
  FeatureAppReview()
      : Feature(FEATURE_APPREVIEW, "App Review",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.5",               // released
                false,               // Can be enabled in devmode
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_IOS) || defined(MVPN_ANDROID)
    return true;
#else
    return false;
#endif
  }

  static const FeatureAppReview* instance() {
    return static_cast<const FeatureAppReview*>(get(FEATURE_APPREVIEW));
  }
};

#endif  // FEATURE_APPREVIEW_H
