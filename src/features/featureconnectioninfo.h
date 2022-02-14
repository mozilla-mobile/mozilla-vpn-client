/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_CONNECTION_INFO_H
#define FEATURE_CONNECTION_INFO_H

#include "models/feature.h"

constexpr const char* FEATURE_CONNECTION_INFO = "connectionInfo";

class FeatureConnectionInfo final : public Feature {
 public:
  FeatureConnectionInfo()
      : Feature(FEATURE_CONNECTION_INFO, "Connection info",
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

  bool checkSupportCallback() const override { return false; }

  static const FeatureConnectionInfo* instance() {
    return static_cast<const FeatureConnectionInfo*>(
        get(FEATURE_CONNECTION_INFO));
  }
};

#endif  // FEATURE_CONNECTION_INFO_H
