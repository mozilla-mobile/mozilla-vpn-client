/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keys.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "hacl-star/Hacl_Curve25519_51.h"
#include "leakdetector.h"

#define CURVE25519_KEY_SIZE 32

Keys::Keys() { MZ_COUNT_CTOR(Keys); }

Keys::~Keys() { MZ_COUNT_DTOR(Keys); }

bool Keys::fromSettings(const QString& privateKey) {
  if (privateKey.isEmpty()) {
    return false;
  }

  // Calculate the corresponding public key.
  QByteArray key = QByteArray::fromBase64(privateKey.toUtf8());
  if (key.length() != CURVE25519_KEY_SIZE) {
    return false;
  }
  QByteArray pubKey(CURVE25519_KEY_SIZE, 0);
  Hacl_Curve25519_51_secret_to_public(reinterpret_cast<uint8_t*>(pubKey.data()),
                                      reinterpret_cast<uint8_t*>(key.data()));

  m_privateKey = privateKey;
  m_publicKey = pubKey.toBase64();
  return true;
}

void Keys::storeKeys(const QString& privateKey, const QString& publicKey) {
  m_privateKey = privateKey;
  m_publicKey = publicKey;
}

void Keys::forgetKeys() {
  m_privateKey.clear();
  m_publicKey.clear();
}
