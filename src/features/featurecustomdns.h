/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_CUSTOM_DNS_H
#define FEATURE_CUSTOM_DNS_H

#include "models/feature.h"
#include "l18nstrings.h"

constexpr const char* FEATURE_CUSTOM_DNS = "customDNS";

class FeatureCustomDNS final : public Feature {
 public:
  FeatureCustomDNS()
      : Feature(FEATURE_CUSTOM_DNS, L18nStrings::FeatureNamesCustomDNS,
                true,                                              // Is Major Feature
                L18nStrings::WhatsNewReleaseNotesDnsModalHeader,   // Display name
                L18nStrings::WhatsNewReleaseNotesDnsListItemBody,  // Description
                L18nStrings::WhatsNewReleaseNotesDnsModalBodyText, // LongDescr
                "../resources/features/custom-dns-preview.png",    // ImagePath
                "../resources/settings/networkSettings.svg",       // IconPath
                "2.5",                                             // released
                true                                               // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_ANDROID) || defined(MVPN_WASM) || defined(MVPN_DUMMY) || \
    defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
    return true;
#else
    return false;
#endif
  }

  static const FeatureCustomDNS* instance() {
    return static_cast<const FeatureCustomDNS*>(get(FEATURE_CUSTOM_DNS));
  }
};

#endif  // FEATURE_CUSTOM_DNS_H
