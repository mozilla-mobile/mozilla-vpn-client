/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURE_SHARE_LOGS_H
#define FEATURE_SHARE_LOGS_H

#include "models/feature.h"

#ifdef MVPN_ANDROID
#  include <QtAndroid>
#endif

constexpr const char* FEATURE_SHARE_LOGS = "shareLogs";
/*
 * This Featureflag describes if the OS can "share" / "open"
 * Logs yielded from the client. This is true for all Desktop OS
 * as we open the logs in the default editor
 * On Mobile this means supporting the "share" functionality.
 * If the feature is not supported a fallback in-app-textview will be used.
 */
class FeatureShareLogs : public Feature {
 public:
  FeatureShareLogs()
      : Feature(FEATURE_SHARE_LOGS, "shareLogs",
                false,               // Is Major Feature
                L18nStrings::Empty,  // Display name
                L18nStrings::Empty,  // Description
                L18nStrings::Empty,  // LongDescr
                "",                  // ImagePath
                "",                  // IconPath
                "2.0",               // released
                false                // Can be enabled in devmode
        ) {}

  bool checkSupportCallback() const override {
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS) || \
    defined(MVPN_IOS)
    return true;
#elif defined(MVPN_ANDROID)
    return QtAndroid::androidSdkVersion() >=
           29;  // Android Q (10) is required for this
#else
    return false;
#endif
  }

  static const FeatureShareLogs* instance() {
    return static_cast<const FeatureShareLogs*>(get(FEATURE_SHARE_LOGS));
  }
};

#endif  // FEATURE_SHARE_LOGS_H
