/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_CAPTIVEPORTAL_H
#define FEATURE_CAPTIVEPORTAL_H

#include "models/feature.h"
#include "l18nstrings.h"

constexpr const char* FEATURE_CAPTIVEPORTAL = "captivePortal";

class FeatureCaptivePortal final : public Feature {
 public:
  FeatureCaptivePortal()
      : Feature(FEATURE_CAPTIVEPORTAL, L18nStrings::FeatureNamesCaptivePortal,
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.1",               // released
                true                 // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_LINUX) || defined(MVPN_MACOS) || defined(MVPN_WINDOWS) || \
    defined(MVPN_DUMMY) || defined(MVPN_WASM)
    return true;
#else
    // If we decide to enable the captive-portal notification for IOS, remember
    // to add the following keys/values in the ios/app/Info.plist:
    // ```
    // <key>NSAppTransportSecurity</key>
    // <dict>
    //   <key>NSAllowsArbitraryLoads</key>
    //   <true/>
    // </dict>
    // ```
    // NSAllowsArbitraryLoads allows the loading of HTTP (not-encrypted)
    // requests. By default, IOS apps work in HTTPS-only mode.
    return false;
#endif
  }

  static const FeatureCaptivePortal* instance() {
    return static_cast<const FeatureCaptivePortal*>(get(FEATURE_CAPTIVEPORTAL));
  }
};

#endif  // FEATURE_CAPTIVEPORTAL_H
