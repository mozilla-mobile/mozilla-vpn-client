/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverhandler.h"
#include "serverconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QHostAddress>
#include <QTcpSocket>

namespace {
Logger logger(LOG_SERVER, "ServerHandler");
}

constexpr int SERVER_PORT = 8754;

ServerHandler::ServerHandler() {
  MVPN_COUNT_CTOR(ServerHandler);

  logger.log() << "Creating the server";

  if (!listen(QHostAddress::LocalHost, SERVER_PORT)) {
    logger.log() << "Failed to listen on port" << SERVER_PORT;
    return;
  }

  connect(this, &ServerHandler::newConnection, this,
          &ServerHandler::newConnectionReceived);
}

ServerHandler::~ServerHandler() { MVPN_COUNT_DTOR(ServerHandler); }

void ServerHandler::newConnectionReceived() {
  QTcpSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  ServerConnection* connection = new ServerConnection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
}
