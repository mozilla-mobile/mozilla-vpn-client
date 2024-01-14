/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDGLEANBRIDGE_H
#define ANDROIDGLEANBRIDGE_H

#include <QObject>

namespace AndroidGleanBridge {

/**
 * @brief Initialize Glean on Android.
 *
 * On Android we initialize Glean from Kotlin.
 *
 * Glean has multiple SDKs. The Android SDK is a fine shell around the Glean
 * Rust SDK. The Mozilla VPN application uses the Glean Rust SDK from the C++
 * side. On Android we custom build the Glean Android SDK and link to the same
 * Glean Rust SDK used on the C++ side.
 *
 * This way we are able to initialize the Glean Android SDK from Kotlin, which
 * has the benefits of a ping uploader that is designed to work well on Android
 * and some other goodies -- but still call the C++ APIs and record to the same
 * Glean instance from both languages.
 *
 * @param isTelemetryEnabled
 * @param channel
 */
void initializeGlean(bool isTelemetryEnabled, const QString& channel);
};  // namespace AndroidGleanBridge

#endif  // ANDROIDGLEANBRIDGE_H
