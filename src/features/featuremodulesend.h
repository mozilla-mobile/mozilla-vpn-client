/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_MODULE_SEND_H
#define FEATURE_MODULE_SEND_H

#include "models/feature.h"

constexpr const char* FEATURE_MODULE_SEND = "moduleSend";

class FeatureModuleSend : public Feature {
 public:
  FeatureModuleSend()
      : Feature(FEATURE_MODULE_SEND, "Module: Send",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.6",               // released
                true                 // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override { return false; }

  static const FeatureModuleSend* instance() {
    return static_cast<const FeatureModuleSend*>(get(FEATURE_MODULE_SEND));
  }
};

#endif  // FEATURE_MODULE_SEND_H
