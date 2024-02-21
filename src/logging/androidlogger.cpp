/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidlogger.h"

#include <QJniObject>

#include "context/androidconstants.h"

// static
bool AndroidLogger::shareText(const QString& text) {
  return (bool)QJniObject::callStaticMethod<jboolean>(
      AndroidConstants::VPN_UTILS_CLASS, "sharePlainText",
      "(Ljava/lang/String;)Z", QJniObject::fromString(text).object());
}
