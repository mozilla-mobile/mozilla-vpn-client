/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhttpserver.h"
#include "inspectorhttpconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QTcpSocket>

namespace {
Logger logger(LOG_INSPECTOR, "InspectorHttpServer");
}

constexpr int INSPECT_PORT = 8766;

InspectorHttpServer::InspectorHttpServer() {
  MVPN_COUNT_CTOR(InspectorHttpServer);

  logger.log() << "Creating the inspector http server";

  if (!listen(QHostAddress::Any, INSPECT_PORT)) {
    logger.log() << "Failed to listen on port" << INSPECT_PORT;
    return;
  }

  connect(this, &InspectorHttpServer::newConnection, this,
          &InspectorHttpServer::newConnectionReceived);
}

InspectorHttpServer::~InspectorHttpServer() {
  MVPN_COUNT_DTOR(InspectorHttpServer);
}

void InspectorHttpServer::newConnectionReceived() {
  QTcpSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  InspectorHttpConnection* connection =
      new InspectorHttpConnection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
}
