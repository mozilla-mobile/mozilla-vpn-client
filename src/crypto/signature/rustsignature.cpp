/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logging/logger.h"
#include "signature.h"

namespace {
Logger logger("Signature");

static void signatureLogger(const char* msg) { logger.debug() << msg; }

}  // namespace

extern "C" {
// Implemented in rust. See the `signature` folder.
// TODO (VPN-5708): We should really generate this with cbindgen.
bool verify_rsa(const char* publicKey, size_t pubKeyLen, const char* message,
                size_t messageLen, const char* signature, size_t signatureLen,
                void (*logfn)(const char*));
};

// static
bool Signature::verifyInternal(const QByteArray& publicKey,
                               const QByteArray& content,
                               const QByteArray& signature) {
  return verify_rsa(publicKey.constData(), publicKey.length(),
                    content.constData(), content.length(),
                    signature.constData(), signature.length(), signatureLogger);
}
