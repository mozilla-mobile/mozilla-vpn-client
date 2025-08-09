/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "httpconnection.h"

#include <QTcpSocket>

#include "dnsresolver.h"

// Peek at the socket and determine if this is a socks connection.
bool HttpConnection::isProxyType(QIODevice* socket) {
  if (!socket->canReadLine()) {
    return false;
  }

  // Read one line and then roll it back.
  socket->startTransaction();
  auto guard = qScopeGuard([socket]() { socket->rollbackTransaction(); });
  QString line = QString(socket->readLine());

  auto match = m_requestRegex.match(line);
  if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
    return false;
  }

  return match.captured(1) == "CONNECT";
}

void HttpConnection::onHttpConnect() {
  // Split the request URI to separate the port and address.
  QStringList list = m_uri.split(':');
  if (list.length() != 2) {
    setError(400, "Bad Request");
    return;
  }

  QString dest = QUrl::fromPercentEncoding(list[0].toUtf8());
  QHostAddress address;
  m_destPort = list[1].toUInt();
  if (address.setAddress(dest)) {
    onHostnameResolved(address);
    return;
  }

  // Perform DNS resolution.
  m_destHostname = dest;
  setState(Resolve);
  DNSResolver::instance()->resolveAsync(m_destHostname, this);
}

void HttpConnection::onHostnameResolved(const QHostAddress& resolved) {
  if (m_destSocket != nullptr) {
    // We might get multiple ip results.
    return;
  }
  Q_ASSERT(!resolved.isNull());

  m_destAddress = resolved;
  m_destSocket = createDestSocket<QTcpSocket>(m_destAddress, m_destPort);

  connect(m_destSocket, &QTcpSocket::connected, this, [this]() {
    sendResponse(200, "OK");

    setState(Proxy);
  });

  connect(m_destSocket, &QTcpSocket::disconnected, this,
          [this]() { setState(Closed); });

  connect(m_destSocket, &QTcpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError error) {
            if (error == QAbstractSocket::RemoteHostClosedError) {
              setState(Closed);
            } else {
              setError(502, "Bad Gateway");
            }
          });

  m_destSocket->connectToHost(m_destAddress, m_destPort);
}
