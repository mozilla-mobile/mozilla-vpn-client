/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hawkauth.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QTextStream>

#include "hkdf.h"

HawkAuth::HawkAuth(const QByteArray& id, const QByteArray& key) {
  m_id = id;
  m_key = key;
  m_timestamp = QDateTime::currentSecsSinceEpoch();
  m_nonce = generateNonce();
}

HawkAuth::HawkAuth(const QByteArray& session) {
  HKDF hash(QCryptographicHash::Sha256);
  hash.addData(session);
  QByteArray keydata =
      hash.result(64, "identity.mozilla.com/picl/v1/sessionToken");

  m_id = keydata.left(32);
  m_key = keydata.right(32);
  m_timestamp = QDateTime::currentSecsSinceEpoch();
  m_nonce = generateNonce();
}

QString HawkAuth::generateNonce() {
  QRandomGenerator* generator = QRandomGenerator::system();
  Q_ASSERT(generator);

  static QByteArray range(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  QString nonce;
  for (int i = 0; i < 6; i++) {
    nonce.append(range.at(generator->generate() % range.length()));
  }
  return nonce;
}

QString HawkAuth::hashPayload(const QByteArray& data, const QString& mimetype) {
  QByteArray hashdata("hawk.1.payload\n");
  hashdata.append(QString(mimetype.toLower() + "\n").toUtf8());
  hashdata.append(data);
  hashdata.append("\n");

  QByteArray digest =
      QCryptographicHash::hash(hashdata, QCryptographicHash::Sha256);
  return QString(digest.toBase64());
}

QString HawkAuth::generateInternal(const QUrl& url, const QString& method,
                                   const QString& hash) {
  int port = (url.scheme() == "http") ? 80 : 443;

  // Build the normalized request header
  QString header = "hawk.1.header\n";
  QTextStream stream(&header);
  stream << QString::number(m_timestamp) << "\n";
  stream << m_nonce << "\n";
  stream << method << "\n";
  if (url.hasQuery()) {
    stream << url.path() << "?" << url.query() << "\n";
  } else {
    stream << url.path() << "\n";
  }
  stream << url.host() << "\n";
  stream << QString::number(url.port(port)) << "\n";
  stream << hash << "\n";
  stream << "\n";

  // Calculate the MAC
  QMessageAuthenticationCode hmac(QCryptographicHash::Sha256, m_key);
  hmac.addData(header.toUtf8());

  // Output the Hawk authentication header
  QStringList values;
  values.append(QString("id=\"%1\"").arg(QString(m_id.toHex())));
  values.append(QString("ts=\"%1\"").arg(m_timestamp));
  values.append(QString("nonce=\"%1\"").arg(m_nonce));
  values.append(QString("mac=\"%1\"").arg(QString(hmac.result().toBase64())));
  if (!hash.isEmpty()) {
    values.append(QString("hash=\"%1\"").arg(hash));
  }

  return QString("Hawk ") + values.join(", ");
}

QString HawkAuth::generate(const QUrl& url, const QString& method,
                           const QString& contentType,
                           const QByteArray& payload) {
  // Hash the payload, if present.
  QString hash;
  if (!payload.isEmpty()) {
    hash = hashPayload(payload, contentType);
  }

  return generateInternal(url, method, hash);
}
