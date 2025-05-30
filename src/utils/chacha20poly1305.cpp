/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chacha20poly1305.h"

#include "hacl-star/Hacl_Chacha20Poly1305_32.h"

Chacha20Poly1305::Chacha20Poly1305(const QByteArray& key) : m_key(key) {
  Q_ASSERT(m_key.length() == KeySize);
}

QByteArray Chacha20Poly1305::encrypt(const QByteArray& nonce,
                                     const QByteArray& aad,
                                     const QByteArray& plaintext,
                                     QByteArray& mac) const {
  Q_ASSERT(nonce.length() == NonceSize);
  
  QByteArray ciphertext(plaintext.length(), 0x00);
  mac.fill(0x00, MacSize);
  Hacl_Chacha20Poly1305_32_aead_encrypt(
      (uint8_t*)m_key.constData(), (uint8_t*)nonce.constData(), aad.length(),
      (uint8_t*)aad.constData(), plaintext.length(),
      (uint8_t*)plaintext.constData(), (uint8_t*)ciphertext.data(),
      (uint8_t*)mac.data());
  return ciphertext;
}

QByteArray Chacha20Poly1305::decrypt(const QByteArray& nonce,
                                     const QByteArray& aad,
                                     const QByteArray& ciphertext,
                                     const QByteArray& mac) const {
  Q_ASSERT(nonce.length() == NonceSize);
  Q_ASSERT(mac.length() == MacSize);

  QByteArray plaintext(ciphertext.length(), 0x00);
  uint32_t err;
  err = Hacl_Chacha20Poly1305_32_aead_decrypt(
      (uint8_t*)m_key.constData(), (uint8_t*)nonce.constData(), aad.length(),
      (uint8_t*)aad.constData(), plaintext.length(), (uint8_t*)plaintext.data(),
      (uint8_t*)ciphertext.constData(), (uint8_t*)mac.constData());
  if (err != 0) {
    return QByteArray();
  } else {
    return plaintext;
  }
}
