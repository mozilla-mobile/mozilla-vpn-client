/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 #ifndef CHACHA20POLY1305_H
 #define CHACHA20POLY1305_H

#include <QByteArray>

class Chacha20Poly1305 final {
 public:
  Chacha20Poly1305(const QByteArray& key);

  static inline constexpr int NonceSize = 12;
  static inline constexpr int MacSize = 16;
  static inline constexpr int KeySize = 32;

  QByteArray encrypt(const QByteArray& nonce, const QByteArray& aad,
                     const QByteArray& plaintext, QByteArray& mac) const;
  QByteArray decrypt(const QByteArray& nonce, const QByteArray& aad,
                     const QByteArray& ciphertext, const QByteArray& mac) const;

 private:
  QByteArray m_key;
};

#endif  // CHACHA20POLY1305_H
