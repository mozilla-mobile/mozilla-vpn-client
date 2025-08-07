/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "proxyconnection.h"

class QIODevice;
class QLocalSocket;
class QTcpSocket;

class HttpConnection final : public ProxyConnection {
  Q_OBJECT

 public:
  explicit HttpConnection(QTcpSocket* socket);
  explicit HttpConnection(QLocalSocket* socket);
  ~HttpConnection() = default;

  // Peek at the socket and determine if this is a HTTP connection.
  static bool isProxyType(QIODevice* socket);
};

#endif  // HTTPCONNECTION_H
