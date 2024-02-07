/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "./server.h"

#include <QDebug>
#include <QHostAddress>
#include <QJsonObject>
#include <QTcpSocket>

#include "connection.h"

constexpr int SERVER_PORT = 8754;

namespace WebExtension {

Server::Server(BaseAdapter* adapter) {
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
  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  return addr == QHostAddress("::ffff:127.0.0.1") ||
         addr == QHostAddress::LocalHost || addr == QHostAddress::LocalHostIPv6;
}

void Server::newConnectionReceived() {
  QTcpSocket* child = nextPendingConnection();

  if (!isAllowedToConnect(child->localAddress())) {
    child->close();
    return;
  }

  Connection* connection = new Connection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
  connect(connection, &Connection::onMessage, m_adapter,
          &BaseAdapter::onMessage);
  connect(m_adapter, &BaseAdapter::onOutgoingMessage, connection,
          &Connection::writeMessage);
}

}  // namespace WebExtension
