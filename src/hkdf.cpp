/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hkdf.h"

#include <QMessageAuthenticationCode>

HKDF::HKDF(QCryptographicHash::Algorithm method, const QByteArray& salt)
    : m_hmac(method, salt) {
  if (salt.size() == 0) {
    m_hmac.setKey(QByteArray(QCryptographicHash::hashLength(method), 0));
  }
  m_algorithm = method;
}

QByteArray HKDF::result(int length, const QByteArray& info) {
  // Compute the internal private key
  QByteArray privkey = m_hmac.result();
  QByteArray block = QByteArray();
  QByteArray result = QByteArray();

  for (char counter = 1; result.size() < length; counter++) {
    // Compute and append the next block
    block.append(info);
    block.append(counter);
    block = QMessageAuthenticationCode::hash(block, privkey, m_algorithm);
    result.append(block);
  }
  return result.left(length);
}
