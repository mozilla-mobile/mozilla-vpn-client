/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "signature.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosutils.h"
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

#ifdef MVPN_SIGNATURE
extern "C" {
// Implemented in rust. See the `signature` folder.
bool verify_rsa(const char* publicKey, size_t pubKeyLen, const char* message,
                size_t messageLen, const char* signature, size_t signatureLen);
};
#endif

// static
bool Signature::verify(const QByteArray& publicKey, const QByteArray& content,
                       const QByteArray& signature) {
  if (signature.isEmpty()) {
    return false;
  }

#if defined(MVPN_SIGNATURE)
  return verify_rsa(publicKey.constData(), publicKey.length(),
                    content.constData(), content.length(),
                    signature.constData(), signature.length());
#elif defined(MVPN_IOS)
  return IOSUtils::verifySignature(publicKey, content, signature);
#elif defined(MVPN_ANDROID)
  return AndroidUtils::verifySignature(publicKey, content, signature);
#elif defined(MVPN_WASM) or defined(UNIT_TEST) or defined(MVPN_DUMMY)
  Q_UNUSED(publicKey);
  Q_UNUSED(content);
  Q_UNUSED(signature);

  return true;
#else
#  error Unsupported platform
#endif
}
