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

  qDebug() << "Got HTTP request:" << line.join(' ');

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

void HttpConnection::handshakeRead() {
  while (m_clientSocket->canReadLine()) {
    // Read the header line.
    if (m_requestLine.isEmpty()) {
      m_requestLine = m_clientSocket->readLine();
      continue;
    }
    
    // Read request headers.
    QString header = m_clientSocket->readLine().trimmed();
    if (!header.isEmpty()) {
      qsizetype sep = header.indexOf(':');
      if ((sep > 0) && (sep+1 < header.length())) {
        m_headers.insert(header.first(sep), header.sliced(sep+1));
        continue;
      } else {
        // TODO: Bad request
        setState(Closed);
        return;
      }
    }

    // Otherwise, an empty line indicates the end of the request headers.
    // TODO: Parse the headers.
    setState(Proxy);
    return;
  }
}

void HttpConnection::clientProxyRead() {
  // TODO:
}
