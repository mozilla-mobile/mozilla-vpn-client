/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "httpconnectionbase.h"

class QIODevice;

class HttpConnection final : public HttpConnectionBase {
  Q_OBJECT

 public:
  explicit HttpConnection(QIODevice* socket) : HttpConnectionBase(socket){};
  ~HttpConnection() = default;

  // Peek at the socket and determine if this is a MASQUE connection.
  static bool isProxyType(QIODevice* socket);

 private slots:
  void onHostnameResolved(const QHostAddress& addr);
  void onHttpConnect();
};

#endif  // HTTPCONNECTION_H
