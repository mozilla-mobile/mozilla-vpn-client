/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "features.h"

#include <QVersionNumber>

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

namespace Feature {

// --- Android features ---

#ifdef MZ_ANDROID
const RuntimeFeature shareLogs = {
    .id = "shareLogs",
    .name = "Share Logs",
    .evaluator = +[] { return AndroidCommons::getSDKVersion() >= 29; },
};
#endif

}  // namespace Feature
