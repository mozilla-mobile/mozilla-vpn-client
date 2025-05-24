/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "signature.h"

#include "logger.h"

#if defined(MZ_SIGNATURE)
#  include "bindings/signature.h"
#endif

#ifdef MZ_SIGNATURE
namespace {
Logger logger("Signature");

static void signatureLogger(const char* msg) { logger.debug() << msg; }

}  // namespace
#endif

// static
bool Signature::verify(const QByteArray& publicKey, const QByteArray& content,
                       const QByteArray& signature) {
  if (signature.isEmpty()) {
    return false;
  }

#if defined(MZ_SIGNATURE)
  auto keyptr = reinterpret_cast<const unsigned char*>(publicKey.constData());
  auto msgptr = reinterpret_cast<const unsigned char*>(content.constData());
  auto sigptr = reinterpret_cast<const unsigned char*>(signature.constData());
  return verify_rsa(keyptr, publicKey.length(), msgptr, content.length(),
                    sigptr, signature.length(), signatureLogger);
#elif defined(MZ_WASM) or defined(UNIT_TEST)
  Q_UNUSED(publicKey);
  Q_UNUSED(content);
  Q_UNUSED(signature);

  return true;
#else
#  error Unsupported platform
#endif
}
