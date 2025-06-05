/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HKDF_H
#define HKDF_H

#include <QByteArray>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>

// HMAC-based Extract-and-Expand Key Derivation Function (HKDF)
// Defined by RFC 5869 and NIST 800-56C
class HKDF final {
 public:
  HKDF(QCryptographicHash::Algorithm method,
       const QByteArray& salt = QByteArray());

  void setSalt(const QByteArray& salt) { m_hmac.setKey(salt); }
  void addData(const char* data, int length) { m_hmac.addData(data, length); }
  void addData(const QByteArray& data) { m_hmac.addData(data); };

  QByteArray result(int length, const QByteArray& info = QByteArray());

 private:
  QCryptographicHash::Algorithm m_algorithm;
  QMessageAuthenticationCode m_hmac;
};

#endif  // HKDF_H
