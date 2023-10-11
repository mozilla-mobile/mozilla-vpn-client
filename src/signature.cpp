/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "signature.h"

#include "logger.h"

#ifdef MZ_IOS
#  include "platforms/ios/ioscommons.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

#ifdef MZ_SIGNATURE
namespace {
Logger logger("Signature");

static void signatureLogger(const char* msg) {
  logger.debug() << msg;
}
}  // namespace

extern "C" {
// Implemented in rust. See the `signature` folder.
bool verify_rsa(const char* publicKey, size_t pubKeyLen, const char* message,
                size_t messageLen, const char* signature, size_t signatureLen,
                void (*logfn)(const char*));
};
#endif

// static
bool Signature::verify(const QByteArray& publicKey, const QByteArray& content,
                       const QByteArray& signature) {
  if (signature.isEmpty()) {
    return false;
  }

#if defined(MZ_SIGNATURE)
  return verify_rsa(publicKey.constData(), publicKey.length(),
                    content.constData(), content.length(),
                    signature.constData(), signature.length(), signatureLogger);
#elif defined(MZ_IOS)
  return IOSCommons::verifySignature(publicKey, content, signature);
#elif defined(MZ_ANDROID)
  return AndroidCommons::verifySignature(publicKey, content, signature);
#elif defined(MZ_WASM) or defined(UNIT_TEST) or defined(MZ_DUMMY)
  Q_UNUSED(publicKey);
  Q_UNUSED(content);
  Q_UNUSED(signature);

  return true;
#else
#  error Unsupported platform
#endif
}
