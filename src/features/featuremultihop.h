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
            true,  // Is Major Feature
            L18nStrings::WhatsNewReleaseNotesMultiModalHeader,   // Display name
            L18nStrings::WhatsNewReleaseNotesMultiListItemBody,  // Description
            L18nStrings::WhatsNewReleaseNotesMultiModalBodyText,  // LongDescr
            "qrc:/ui/resources/features/multi-hop-preview.png",   // ImagePath
            "qrc:/nebula/resources/location-dark.svg",            // IconPath
            "",                                                   // link URL
#if defined(MVPN_ANDROID) || defined(MVPN_IOS)
            "2.7",  // released for android
#else
            "2.5",  // released for desktop
#endif
            true,          // Can be enabled in devmode
            QStringList()  // feature dependencies
        ) {
  }

  bool checkSupportCallback() const override { return true; }

  static const FeatureMultiHop* instance() {
    return static_cast<const FeatureMultiHop*>(get(FEATURE_MULTI_HOP));
  }
};

#endif  // FEATURE_MULTI_HOP_H
