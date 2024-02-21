/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HAWKAUTH_H
#define HAWKAUTH_H

#include <QByteArray>
#include <QNetworkRequest>
#include <QString>

class HawkAuth final {
 public:
  HawkAuth(const QByteArray& session);
  HawkAuth(const QByteArray& id, const QByteArray& key);

  QString generate(const QUrl& url, const QString& method,
                   const QString& contentType,
                   const QByteArray& payload = QByteArray());

  static QString hashPayload(const QByteArray& data, const QString& mimetype);

 private:
  QString generateInternal(const QUrl& url, const QString& method,
                           const QString& hash);

  static QString generateNonce();

  qint64 m_timestamp;
  QString m_nonce;
  QByteArray m_id;
  QByteArray m_key;
};

#endif  // HAWKAUTH_H
