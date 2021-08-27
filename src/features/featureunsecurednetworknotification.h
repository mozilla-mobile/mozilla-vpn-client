/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_UNSECURED_NETWORK_H
#define FEATURE_UNSECURED_NETWORK_H

#include "models/feature.h"
#include "l18nstrings.h"

constexpr const char* FEATURE_UNSECUREDNETWORKNOTIFICATION =
    "unsecuredNetworkNotification";

class FeatureUnsecuredNetworkNotification final : public Feature {
 public:
  FeatureUnsecuredNetworkNotification()
      : Feature(FEATURE_UNSECUREDNETWORKNOTIFICATION,
                L18nStrings::FeatureNamesUnsecuredNetworkNotification,
                false,               // Is Major Feature
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.2",               // released
                true                 // Can be enabled in devmode
        ){};

  bool checkSupportCallback() const override {
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS) || \
    defined(MVPN_WASM) || defined(MVPN_DUMMY)
    return true;
#else
    return false;
#endif
  }

  static const FeatureUnsecuredNetworkNotification* instance() {
    return static_cast<const FeatureUnsecuredNetworkNotification*>(
        get(FEATURE_UNSECUREDNETWORKNOTIFICATION));
  }
};

#endif  // FEATURE_UNSECURED_NETWORK_H
