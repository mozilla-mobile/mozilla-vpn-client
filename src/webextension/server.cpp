/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "./server.h"

#include <QHostAddress>
#include <QTcpSocket>

#include "./connection.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("Server");
}

constexpr int SERVER_PORT = 8754;

namespace WebExtension {

Server::Server() {
  MZ_COUNT_CTOR(Server);

  logger.debug() << "Creating the server";

  if (!listen(QHostAddress::LocalHost, SERVER_PORT)) {
    logger.error() << "Failed to listen on port" << SERVER_PORT;
    return;
  }

  connect(this, &Server::newConnection, this, &Server::newConnectionReceived);
}

Server::~Server() { MZ_COUNT_DTOR(Server); }

void Server::newConnectionReceived() {
  QTcpSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  Connection* connection = new Connection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
}

}  // namespace WebExtension