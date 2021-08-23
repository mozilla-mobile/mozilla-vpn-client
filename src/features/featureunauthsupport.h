/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_UNAUTH_SUPPORT_H
#define FEATURE_UNAUTH_SUPPORT_H

#include "models/feature.h"
#include "l18nstrings.h"

constexpr const char* FEATURE_UNAUTH_SUPPORT = "unauthSupport";

class FeatureUnauthSupport final : public Feature {
 public:
  FeatureUnauthSupport()
      : Feature(FEATURE_UNAUTH_SUPPORT, L18nStrings::FeatureNamesUnauthSupport,
                true,                                                  // Is Major Feature
                L18nStrings::WhatsNewReleaseNotesSupportListItemBody,  // Description
                L18nStrings::WhatsNewReleaseNotesSupportModalBodyText, // LongDescr
                "../resources/support-form-preview.png",               // ImagePath
                "../resources/settings/getHelp.svg",                   // IconPath
                "2.5",                                                 // released
                true                                                   // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override { return m_supported; }

  static FeatureUnauthSupport* instance() {
    return (FeatureUnauthSupport*)(get(FEATURE_UNAUTH_SUPPORT));
  }

  void setIsSupported(bool enabled) { m_supported = enabled; }
              
 private:
  bool m_supported = false;
};

#endif  // FEATURE_UNAUTH_SUPPORT_H
