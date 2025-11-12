/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "server.h"

#include <QDebug>
#include <QHostAddress>
#include <QJsonObject>
#include <QLocalSocket>

#include "connection.h"

constexpr const char* WEBEXT_SERVER_NAME = "mozillavpn.webext";

namespace WebExtension {

Server::Server(BaseAdapter* adapter) : QLocalServer(adapter) {
  Q_ASSERT(adapter);
  m_adapter = adapter;

  qInfo() << "Creating the server";

  setSocketOptions(QLocalServer::UserAccessOption);
  if (!listen(WEBEXT_SERVER_NAME)) {
    qCritical() << "Failed to listen on name" << WEBEXT_SERVER_NAME;
    return;
  }
  qInfo() << "Server name:" << fullServerName();

  connect(this, &Server::newConnection, this, &Server::newConnectionReceived);
}

Server::~Server() {}

void Server::newConnectionReceived() {
  QLocalSocket* child = nextPendingConnection();

  if (!isAllowedToConnect(child->socketDescriptor())) {
    qInfo() << "Refused connection.";
    child->close();
    return;
  }

  Connection* connection = new Connection(this, child);
  connect(child, &QLocalSocket::disconnected, connection,
          &QObject::deleteLater);
  connect(connection, &Connection::onMessageReceived, m_adapter,
          &BaseAdapter::onMessage);
  connect(m_adapter, &BaseAdapter::onOutgoingMessage, connection,
          &Connection::writeMessage);
}

}  // namespace WebExtension
