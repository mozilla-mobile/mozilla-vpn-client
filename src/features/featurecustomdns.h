/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_CUSTOM_DNS_H
#define FEATURE_CUSTOM_DNS_H

#include "models/feature.h"

constexpr const char* FEATURE_CUSTOM_DNS = "customDNS";

class FeatureCustomDNS final : public Feature {
 public:
  FeatureCustomDNS()
      : Feature(
            FEATURE_CUSTOM_DNS, "Custom DNS",
            true,  // Is Major Feature
            L18nStrings::WhatsNewReleaseNotesDnsModalHeader,   // Display name
            L18nStrings::WhatsNewReleaseNotesDnsListItemBody,  // Description
            L18nStrings::WhatsNewReleaseNotesDnsModalBodyTextNew,  // LongDescr
            "qrc:/ui/resources/features/custom-dns-preview.png",   // ImagePath
            "qrc:/ui/resources/settings/networkSettings.svg",      // IconPath
            "",                                                    // link URL
            "2.5",                                                 // released
            true,          // Can be enabled in devmode
            QStringList()  // feature dependencies
        ) {}

  bool checkSupportCallback() const override { return true; }

  static const FeatureCustomDNS* instance() {
    return static_cast<const FeatureCustomDNS*>(get(FEATURE_CUSTOM_DNS));
  }
};

#endif  // FEATURE_CUSTOM_DNS_H
