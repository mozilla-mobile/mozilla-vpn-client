/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "httpconnection.h"

#include <QRegularExpression>
#include <QLocalSocket>
#include <QTcpSocket>

HttpConnection::HttpConnection(QTcpSocket* sock) : ProxyConnection(sock) {}
HttpConnection::HttpConnection(QLocalSocket* sock) : ProxyConnection(sock) {}

// Peek at the socket and determine if this is a socks connection.
bool HttpConnection::isProxyType(QIODevice* socket) {
  if (!socket->canReadLine()) {
    return false;
  }

  // Read one line and then roll it back.
  socket->startTransaction();
  auto guard = qScopeGuard([socket](){ socket->rollbackTransaction(); });
  QStringList line = QString(socket->readLine()).split(' ');

  // There should be three tokens:
  //  Method
  //  Request-URI
  //  HTTP-Version
  static const QRegularExpression nonUpperAlpha("[^A-Z]");
  static const QRegularExpression httpVersion("HTTP/[0-9].[0-9]");
  if (line.length() != 3) {
    return false;
  }
  if (!line.at(0).contains(nonUpperAlpha)) {
    return false;
  }
  return line.at(3).indexOf(httpVersion) == 0;
}
