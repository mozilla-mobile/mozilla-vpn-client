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
      : Feature(
            FEATURE_CONNECTION_INFO, "Connection info",
            true,  // Is Major Feature
            L18nStrings::WhatsNewReleaseNotesConnectionInfoDisplayName,  // Display
                                                                         // name
            L18nStrings::
                WhatsNewReleaseNotesConnectionInfoDescriptionShort,  // Description
            L18nStrings::
                WhatsNewReleaseNotesConnectionInfoDescription,  // LongDescr
            "qrc:/ui/resources/features/connection-info-preview.png",  // ImagePath
            "qrc:/ui/resources/features/connection-info-icon.svg",  // IconPath
            "",                                                     // link URL
            "2.8",                                                  // released
            true  // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override { return true; }

  static const FeatureConnectionInfo* instance() {
    return static_cast<const FeatureConnectionInfo*>(
        get(FEATURE_CONNECTION_INFO));
  }
};

#endif  // FEATURE_CONNECTION_INFO_H
