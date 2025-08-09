/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QMap>
#include <QUrl>

#include "proxyconnection.h"

class QIODevice;

class HttpConnection : public ProxyConnection {
  Q_OBJECT

 public:
  explicit HttpConnection(QIODevice* socket);
  ~HttpConnection() = default;

  // Peek at the socket and determine if this is a HTTP connection.
  static bool isProxyType(QIODevice* socket);

  void handshakeRead() override;

 private slots:
  void onHostnameResolved(const QHostAddress& addr);
  void onHostnameNotFound();
  void onHttpConnect();

 protected:
  void sendResponse(
      int code, const QString& message,
      const QMap<QString, QString>& hdr = QMap<QString, QString>());
  void setError(int code, const QString& message);
  const QString& header(const QString& name) const;

  // HTTP Request fields.
  QUrl m_baseUrl;
  QString m_method;
  QString m_uri;
  QString m_version;
  QMap<QString, QString> m_headers;

  static const QRegularExpression m_requestRegex;
};

#endif  // HTTPCONNECTION_H
