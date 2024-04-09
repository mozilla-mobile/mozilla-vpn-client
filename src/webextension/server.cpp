/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "server.h"

#include <QDebug>
#include <QHostAddress>
#include <QJsonObject>
#include <QTcpSocket>

#include "connection.h"

constexpr int SERVER_PORT = 8754;
// `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
// notation.
constexpr auto LOCALHOST_V4_AS_V6 = "::ffff:127.0.0.1";

namespace WebExtension {

Server::Server(BaseAdapter* adapter) : QTcpServer(adapter) {
  Q_ASSERT(adapter);
  m_adapter = adapter;

  qInfo() << "Creating the server";

  if (!listen(QHostAddress::LocalHost, SERVER_PORT)) {
    qCritical() << "Failed to listen on port" << SERVER_PORT;
    return;
  }

  connect(this, &Server::newConnection, this, &Server::newConnectionReceived);
}

Server::~Server() {}

bool Server::isAllowedToConnect(QHostAddress addr) {
  return addr == QHostAddress(LOCALHOST_V4_AS_V6) ||
         addr == QHostAddress::LocalHost || addr == QHostAddress::LocalHostIPv6;
}

void Server::newConnectionReceived() {
  QTcpSocket* child = nextPendingConnection();

  if (!isAllowedToConnect(child->localAddress())) {
    qInfo() << "Refused connection from non-localhost address.";
    child->close();
    return;
  }

  Connection* connection = new Connection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
  connect(connection, &Connection::onMessageReceived, m_adapter,
          &BaseAdapter::onMessage);
  connect(m_adapter, &BaseAdapter::onOutgoingMessage, connection,
          &Connection::writeMessage);
}

}  // namespace WebExtension
