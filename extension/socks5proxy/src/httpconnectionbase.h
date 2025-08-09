/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HTTPCONNECTIONBASE_H
#define HTTPCONNECTIONBASE_H

#include <QMap>
#include <QUrl>

#include "httprequest.h"
#include "proxyconnection.h"

class QIODevice;

class HttpConnectionBase : public ProxyConnection {
  Q_OBJECT

 public:
  explicit HttpConnectionBase(QIODevice* socket);
  ~HttpConnectionBase() = default;

  void handshakeRead() override;

 private slots:
  void onHostnameNotFound();

 protected:
  void sendResponse(
      int code, const QString& message,
      const QMap<QString, QString>& hdr = QMap<QString, QString>());
  void setError(int code, const QString& message);

  // HTTP Request fields.
  QUrl m_baseUrl;
  HttpRequest m_request;
};

#endif  // HTTPCONNECTIONBASE_H
