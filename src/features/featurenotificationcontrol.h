/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_NOTIFICATION_CONTROL_H
#define FEATURE_NOTIFICATION_CONTROL_H

#include "models/feature.h"

constexpr const char* FEATURE_NOTIFICATION_CONTROL = "notificationControl";

class FeatureNotificationControl final : public Feature {
 public:
  FeatureNotificationControl()
      : Feature(FEATURE_NOTIFICATION_CONTROL, "Notification control",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.3",               // released
                true                 // Can be enabled in devmode
        ){};

  bool checkSupportCallback() const override { return true; }

  static const FeatureNotificationControl* instance() {
    return static_cast<const FeatureNotificationControl*>(
        get(FEATURE_NOTIFICATION_CONTROL));
  }
};

#endif  // FEATURE_NOTIFICATION_CONTROL_H
