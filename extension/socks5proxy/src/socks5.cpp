/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>

#include "httpconnection.h"
#include "socks5connection.h"

#define MAX_CLIENTS 1024

Socks5::Socks5(QLocalServer* server) : QObject(server) {
  connect(server, &QLocalServer::newConnection, this,
          [this, server]() { newConnection(server); });
}

Socks5::Socks5(QTcpServer* server) : QObject(server) {
  connect(server, &QTcpServer::newConnection, this,
          [this, server]() { newConnection(server); });
}

Socks5::~Socks5() { m_shuttingDown = true; }

template <typename T>
void Socks5::newConnection(T* server) {
  while (server->hasPendingConnections() && (m_clientCount < MAX_CLIENTS)) {
    auto* socket = server->nextPendingConnection();
    if (!socket) {
      return;
    }
    if constexpr (std::is_same_v<T, QTcpSocket>) {
      socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
      socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    }

    // Destroy this socket if it disconnects before we can figure out what
    // protocol is being used.
    QTimer* timer = new QTimer(socket);
    timer->setObjectName("self-destruct-timer");
    timer->setSingleShot(true);
    timer->start(3000);
    connect(timer, &QTimer::timeout, socket, &QObject::deleteLater);
    connect(socket, SIGNAL(disconnected()), this, SLOT(discardSocket()));

    tryCreateProxy(socket);
  }
}

void Socks5::tryCreateProxy(QIODevice* socket) {
  // Inspect the socket to see what kind of protocol its using.
  ProxyConnection* con = nullptr;
  if (HttpConnection::isProxyType(socket)) {
    con = new HttpConnection(socket);
  } else if (Socks5Connection::isProxyType(socket)) {
    con = new Socks5Connection(socket);
  } else if (socket->bytesAvailable() > 4096) {
    // If we haven't figured it out by now, we can give up on this connection.
    socket->deleteLater();
    return;
  } else {
    // Otherwise, wait for more bytes.
    Qt::ConnectionType ctype = Qt::SingleShotConnection;
    connect(socket, &QIODevice::readyRead, this,
            [this, socket](){ tryCreateProxy(socket); }, ctype);
    return;
  }

  // Stop the self-destruct-timer
  QTimer* timer = socket->findChild<QTimer*>("self-destruct-timer",
                                             Qt::FindDirectChildrenOnly);
  if (timer) {
    timer->stop();
    timer->deleteLater();
  }

  connect(con, &QObject::destroyed, this, &Socks5::clientDismissed);
  connect(con, &Socks5Connection::setupOutSocket, this,
          [this](qintptr sd, const QHostAddress& dest) {
            emit outgoingConnection(sd, dest);
          });

  ++m_clientCount;
  emit incomingConnection(con);
  emit connectionsChanged();
}

// This signal is called to discard a socket before we can determine its type.
void Socks5::discardSocket() {
  // Fire the self-destruct-timer immediately.
  QObject* obj = QObject::sender();
  QTimer* timer = obj->findChild<QTimer*>("self-destruct-timer",
                                          Qt::FindDirectChildrenOnly);
  if (timer) {
    timer->start(0);
  }
}

void Socks5::clientDismissed() {
  Q_ASSERT(m_clientCount > 0);

  if (!m_shuttingDown) {
    --m_clientCount;
    emit connectionsChanged();
  }
}
