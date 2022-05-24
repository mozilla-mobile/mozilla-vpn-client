/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_ACCOUNT_DELETION_H
#define FEATURE_ACCOUNT_DELETION_H

#include "models/feature.h"

constexpr const char* FEATURE_ACCOUNT_DELETION = "accountDeletion";

class FeatureAccountDeletion final : public Feature {
 public:
  FeatureAccountDeletion()
      : Feature(FEATURE_ACCOUNT_DELETION, "Account deletion",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.9",               // released
                true,                // Can be flipped on
                false,               // Can be flipped off
                QStringList()        // feature dependencies
        ) {}

  bool checkSupportCallback() const override {
    return false;
  }

  static const FeatureAccountDeletion* instance() {
    return static_cast<const FeatureAccountDeletion*>(
        get(FEATURE_ACCOUNT_DELETION));
  }
};

#endif  // FEATURE_ACCOUNT_DELETION_H
