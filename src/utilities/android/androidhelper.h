/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDHELPER_H
#define ANDROIDHELPER_H

#include <jni.h>

#include <QByteArray>

/**
 * @brief Collection of helper function for Android.
 *
 */
namespace AndroidHelper {

/**
 * @brief Creates a copy of the passed QByteArray in the JVM and passes back a
 * ref
 *
 * @param data
 * @return jbyteArray
 */
jbyteArray tojByteArray(const QByteArray& data);
}  // namespace AndroidHelper

#endif  // ANDROIDHELPER_H
