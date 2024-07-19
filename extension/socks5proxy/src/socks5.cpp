/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5.h"

#include "socks5connection.h"

#define MAX_CLIENTS 1024

Socks5::Socks5(uint16_t port, QHostAddress listenAddress = QHostAddress::Any,
               QObject* parent = nullptr)
    : QObject(parent) {
  connect(&m_server, &QTcpServer::newConnection, this, &Socks5::newConnection);
  qDebug() << "port" << port;
  if (!m_server.listen(listenAddress, port)) {
    qDebug() << "Unable to listen to the proxy port" << port;
    return;
  }
}

Socks5::~Socks5() { m_shuttingDown = true; }

void Socks5::newConnection() {
  while (m_server.hasPendingConnections() && m_clientCount < MAX_CLIENTS) {
    QTcpSocket* socket = m_server.nextPendingConnection();
    if (!socket) {
      return;
    }

    emit incomingConnection(socket, socket->peerAddress());

    auto const con = new Socks5Connection(socket, m_server.serverPort());
    connect(con, &QObject::destroyed, this, &Socks5::clientDismissed);
    connect(con, &Socks5Connection::dataSentReceived,
            [this](qint64 sent, qint64 received) {
              emit dataSentReceived(sent, received);
            });

    connect(con, &Socks5Connection::setupOutSocket, this,
            [this](QAbstractSocket* s, const QHostAddress& dest) {
              emit outgoingConnection(s, dest);
            });

    ++m_clientCount;
    emit connectionsChanged();
  }
}

void Socks5::clientDismissed() {
  Q_ASSERT(m_clientCount > 0);

  if (!m_shuttingDown) {
    --m_clientCount;
    emit connectionsChanged();

    newConnection();
  }
}

int Socks5::port() const { return m_server.serverPort(); }
