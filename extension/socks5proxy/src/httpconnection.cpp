/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "httpconnection.h"

#include <QBuffer>
#include <QRegularExpression>
#include <QLocalSocket>
#include <QTcpSocket>
#include <QUrl>

#include "dnsresolver.h"

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

  return true;
}

void HttpConnection::sendResponse(int code, const QString& message,
                                  const QMap<QString,QString>& headers) {
  QString status = QString("HTTP/1.1 %1 %2\r\n").arg(code).arg(message);
  QByteArray response = status.toUtf8();

  for (auto i = headers.cbegin(); i != headers.cend(); i++) {
    QString header = QString("%1: %2\r\n").arg(i.key()).arg(i.value());
    response.append(header.toUtf8());
  }
  response.append("\r\n");

  m_clientSocket->write(response);
}

void HttpConnection::setError(int code, const QString& message) {
  if (m_state != Proxy) {
    sendResponse(code, message);
  }

  m_errorString = message;
  setState(Closed);
}

void HttpConnection::handshakeRead() {
  while (m_clientSocket->canReadLine()) {
    QString header = m_clientSocket->readLine().trimmed();

    // Read the header line.
    if (m_method.isEmpty()) {
      auto match = m_requestRegex.match(header);
      if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
        setError(400, "Bad Request");
        return;
      }

      m_method = match.captured(1);
      m_uri = match.captured(2);
      m_version = match.captured(3);
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
    if (m_method == "CONNECT") {
      doConnect();
    } else {
      setError(405, "Method Not Allowed");
    }
    return;
  }
}

void HttpConnection::doConnect() {
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
  m_destSocket = createDestSocket(m_destAddress, m_destPort);

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
}

void HttpConnection::onHostnameNotFound() {
  setError(404, "Not Found");
}
