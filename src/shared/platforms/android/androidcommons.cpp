/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidcommons.h"

#include <jni.h>

#include <QJniEnvironment>
#include <QJniObject>

#include "logger.h"

constexpr auto COMMON_UTILS_CLASS = "org/mozilla/firefox/qt/common/Utils";

namespace {
Logger logger("AndroidCommons");

// Creates a copy of the passed QByteArray in the JVM and passes back a ref
jbyteArray tojByteArray(const QByteArray& data) {
  QJniEnvironment env;
  jbyteArray out = env->NewByteArray(data.size());
  env->SetByteArrayRegion(out, 0, data.size(),
                          reinterpret_cast<const jbyte*>(data.constData()));
  return out;
}

}  // namespace

// static
bool AndroidCommons::verifySignature(const QByteArray& publicKey,
                                     const QByteArray& content,
                                     const QByteArray& signature) {
  QJniEnvironment env;
  auto out = (bool)QJniObject::callStaticMethod<jboolean>(
      COMMON_UTILS_CLASS, "verifyContentSignature", "([B[B[B)Z",
      tojByteArray(publicKey), tojByteArray(content), tojByteArray(signature));
  logger.info() << "Android Signature Response" << out;
  return out;
}
