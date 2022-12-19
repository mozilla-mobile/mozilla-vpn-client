/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "curve25519.h"

#include "hacl-star/Hacl_Curve25519_51.h"

// static
QByteArray Curve25519::generatePublicKey(const QByteArray& privateKey) {
  QByteArray key = QByteArray::fromBase64(privateKey);

  Q_ASSERT(key.length() == CURVE25519_KEY_SIZE);
  uint8_t privKey[CURVE25519_KEY_SIZE];
  for (int i = 0; i < CURVE25519_KEY_SIZE; ++i) {
    privKey[i] = (uint8_t)key.at(i);
  }

  uint8_t pubKey[CURVE25519_KEY_SIZE];
  Hacl_Curve25519_51_secret_to_public(pubKey, privKey);

  QByteArray pk =
      QByteArray::fromRawData((const char*)pubKey, CURVE25519_KEY_SIZE);
  return pk.toBase64();
}
