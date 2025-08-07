/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "httpconnection.h"

#include <QRegularExpression>
#include <QLocalSocket>
#include <QTcpSocket>
#include <QUrl>

#include "dnsresolver.h"

HttpConnection::HttpConnection(QTcpSocket* sock) : ProxyConnection(sock) {}
HttpConnection::HttpConnection(QLocalSocket* sock) : ProxyConnection(sock) {}

const QRegularExpression HttpConnection::m_requestRegex =
    QRegularExpression("^([A-Z-]+)\\s([^\\s]+)*\\sHTTP/([0-9].[0-9])");

// Peek at the socket and determine if this is a socks connection.
bool HttpConnection::isProxyType(QIODevice* socket) {
  if (!socket->canReadLine()) {
    return false;
  }

  // Read one line and then roll it back.
  socket->startTransaction();
  auto guard = qScopeGuard([socket](){ socket->rollbackTransaction(); });
  QString line = QString(socket->readLine());

  auto match = m_requestRegex.match(line);
  if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
    return false;
  }

  qDebug() << "Got HTTP request:" << match.captured(0);
  qDebug() << "   method:" << match.captured(1);
  qDebug() << "   uri:   " << match.captured(2);
  qDebug() << "   version" << match.captured(3);
  return true;
}

void HttpConnection::setError(int code, const QString& message) {
  if (m_state != Proxy) {
    QString response = QString("HTTP/1.1 %1 %2\r\n\r\n").arg(code).arg(message);
    m_clientSocket->write(response.toUtf8());
  }

  m_errorString = message;
  setState(Closed);
}

void HttpConnection::handshakeRead() {
  while (m_clientSocket->canReadLine()) {
    QString header = m_clientSocket->readLine().trimmed();

    // Read the header line.
    if (m_requestMethod.isEmpty()) {
      auto match = m_requestRegex.match(header);
      if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
        setError(400, "Bad Request");
        return;
      }

      m_requestMethod = match.captured(1);
      m_requestUri = match.captured(2);
      m_requestVersion = match.captured(3);
      continue;
    }

    // Read request headers.
    if (!header.isEmpty()) {
      qsizetype sep = header.indexOf(':');
      if ((sep > 0) && (sep+1 < header.length())) {
        m_headers.insert(header.first(sep), header.sliced(sep+1));
        continue;
      } else {
        setError(400, "Bad Request");
        return;
      }
    }

    // Handle the request method.
    if (m_requestMethod == "CONNECT") {
      doConnect();
    } else {
      setError(405, "Method Not Allowed");
    }
    return;
  }
}

void HttpConnection::doConnect() {
  // Split the request URI to separate the port and address.
  QStringList list = m_requestUri.split(':');
  if (list.length() != 2) {
    setError(400, "Bad Request");
    return;
  }

  QString dest = QUrl::fromPercentEncoding(list[1].toUtf8());
  QHostAddress addr = QHostAddress(dest);
  m_destPort = list[1].toUInt();
  if (addr.isNull()) {
    // Perform DNS resolution.
    m_destHostname = dest;
    setState(Resolve);
    DNSResolver::instance()->resolveAsync(m_destHostname, this);
    return;
  }

  onHostnameResolved(addr);

}

void HttpConnection::onHostnameResolved(const QHostAddress& resolved) {
  if (m_destSocket != nullptr) {
    // We might get multiple ip results.
    return;
  }
  Q_ASSERT(!resolved.isNull());

  // Otherwise, connect to the destination.
  m_destAddress = resolved;
  m_destSocket = createDestSocket(m_destAddress, m_destPort);

  connect(m_destSocket, &QTcpSocket::connected, this, [this]() {
    setError(200, "OK");

    // Keep track of how many bytes are yet to be written to finish the
    // negotiation. We should suppress the statistics signals for such traffic.
    m_recvIgnoreBytes = m_clientSocket->bytesToWrite();
  
    setState(Proxy);
    clientReadyRead();
  });
}

void HttpConnection::onHostnameNotFound() {
  setError(404, "Not Found");
}
