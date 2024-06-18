/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5.h"

#include "socks5client.h"

#define MAX_CLIENTS 1024

Socks5::Socks5(QObject* parent, uint16_t port) : QObject(parent) {
  connect(&m_server, &QTcpServer::newConnection, this, &Socks5::newConnection);

  if (!m_server.listen(QHostAddress::Any, port)) {
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

    emit incomingConnection(socket->peerAddress().toString());

    new Socks5Client(this, socket, m_server.serverPort());
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
