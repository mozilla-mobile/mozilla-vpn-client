/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>

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

    auto const con = new Socks5Connection(socket);
    connect(con, &QObject::destroyed, this, [this, server]() {
      clientDismissed();
      newConnection(server);
    });
    connect(con, &Socks5Connection::dataSentReceived,
            [this](qint64 sent, qint64 received) {
              emit dataSentReceived(sent, received);
            });

    connect(con, &Socks5Connection::setupOutSocket, this,
            [this](QAbstractSocket* s, const QHostAddress& dest) {
              emit outgoingConnection(s, dest);
            });

    ++m_clientCount;
    emit incomingConnection(con);
    emit connectionsChanged();
  }
}

void Socks5::clientDismissed() {
  Q_ASSERT(m_clientCount > 0);

  if (!m_shuttingDown) {
    --m_clientCount;
    emit connectionsChanged();
  }
}
