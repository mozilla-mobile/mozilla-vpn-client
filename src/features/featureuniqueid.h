/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_UNIQUE_ID_H
#define FEATURE_UNIQUE_ID_H

#include "constants.h"
#include "models/feature.h"

constexpr const char* FEATURE_UNIQUE_ID = "shareLogs";
/*
 * This Featureflag describes if the Client
 * creates a guardian Device using it's unique ID
 */
class FeatureUniqueID : public Feature {
 public:
  FeatureUniqueID()
      : Feature(FEATURE_UNIQUE_ID, "uniqueID",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.6",               // released
                false                // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
        // This feature can't be enabled in production: 
        // We need to solve the underlying guardian issue,
        // See: https://mozilla-hub.atlassian.net/browse/VPN-1177
      return !Constants::inProduction();
  }

  static const FeatureUniqueID* instance() {
    return static_cast<const FeatureUniqueID*>(get(FEATURE_UNIQUE_ID));
  }
};

#endif  // FEATURE_UNIQUE_ID_H
