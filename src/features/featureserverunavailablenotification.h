/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_SERVER_UNAVAILABLE_NOTIFICATION_H
#define FEATURE_SERVER_UNAVAILABLE_NOTIFICATION_H

#include "models/feature.h"

constexpr const char* FEATURE_SERVERUNAVAILABLENOTIFICATION =
    "serverUnavailableNotification";

class FeatureServerUnavailableNotification final : public Feature {
 public:
  FeatureServerUnavailableNotification()
      : Feature(FEATURE_SERVERUNAVAILABLENOTIFICATION,
                "Server unavailable notification",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "",                  // link URL
                "2.7",               // released
                true,                // Can be enabled in devmode
                QStringList()        // feature dependencies
        ){};

  bool checkSupportCallback() const override { return true; }

  static const FeatureServerUnavailableNotification* instance() {
    return static_cast<const FeatureServerUnavailableNotification*>(
        get(FEATURE_SERVERUNAVAILABLENOTIFICATION));
  }
};

#endif  // FEATURE_SERVER_UNAVAILABLE_NOTIFICATION_H
