/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_MULTI_HOP_H
#define FEATURE_MULTI_HOP_H

#include "models/feature.h"

constexpr const char* FEATURE_MULTI_HOP = "multiHop";

class FeatureMultiHop : public Feature {
 public:
  FeatureMultiHop()
      : Feature(
            FEATURE_MULTI_HOP, "Multi-hop",
            true,                                                 // Is Major Feature
            L18nStrings::WhatsNewReleaseNotesMultiModalHeader,    // Display name
            L18nStrings::WhatsNewReleaseNotesMultiListItemBody,   // Description
            L18nStrings::WhatsNewReleaseNotesMultiModalBodyText,  // LongDescr
            "../resources/features/multi-hop-preview.png",        // ImagePath
            "../resources/location-dark.svg",                     // IconPath
            "2.5",                                                // released
            true                                                  // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS) || defined(MVPN_DUMMY) || \
    defined(MVPN_MACOS_DAEMON)
    return true;
#else
    return false;
#endif
  }

  static const FeatureMultiHop* instance() {
    return static_cast<const FeatureMultiHop*>(get(FEATURE_MULTI_HOP));
  }
};

#endif  // FEATURE_MULTI_HOP_H
