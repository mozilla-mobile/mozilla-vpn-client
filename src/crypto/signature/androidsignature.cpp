/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <jni.h>

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QTimer>

#include "context/androidconstants.h"
#include "logging/logger.h"
#include "settings/settingsholder.h"
#include "signature.h"
#include "utilities/android/androidhelper.h"

namespace {
Logger logger("AndroidVerifySignature");

}  // namespace

bool Signature::verifyInternal(const QByteArray& publicKey,
                               const QByteArray& content,
                               const QByteArray& signature) {
  QJniEnvironment env;
  auto out = (bool)QJniObject::callStaticMethod<jboolean>(
      AndroidConstants::COMMON_UTILS_CLASS, "verifyContentSignature",
      "([B[B[B)Z", AndroidUtils::tojByteArray(publicKey), tojByteArray(content),
      tojByteArray(signature));
  logger.info() << "Android Signature Response" << out;
  return out;
}
