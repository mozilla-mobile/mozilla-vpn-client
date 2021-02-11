/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorwebsocketserver.h"
#include "inspectorwebsocketconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QWebSocket>

namespace {
Logger logger(LOG_INSPECTOR, "InspectorWebSocketServer");
}

constexpr int INSPECT_PORT = 8765;

InspectorWebSocketServer::InspectorWebSocketServer()
    : QWebSocketServer("", QWebSocketServer::NonSecureMode) {
  MVPN_COUNT_CTOR(InspectorWebSocketServer);

  logger.log() << "Creating the inspector websocket server";

  if (!listen(QHostAddress::Any, INSPECT_PORT)) {
    logger.log() << "Failed to listen on port" << INSPECT_PORT;
    return;
  }

  connect(this, &InspectorWebSocketServer::newConnection, this,
          &InspectorWebSocketServer::newConnectionReceived);
}

InspectorWebSocketServer::~InspectorWebSocketServer() {
  MVPN_COUNT_DTOR(InspectorWebSocketServer);
}

void InspectorWebSocketServer::newConnectionReceived() {
  QWebSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  InspectorWebSocketConnection* connection =
      new InspectorWebSocketConnection(this, child);
  connect(child, &QWebSocket::disconnected, connection, &QObject::deleteLater);
}
