/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocket>
#include <qapplication.h>

#include "../inspector.h"
#include "websockettransport.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("InspectorWebSocketServer");
}

namespace InspectorServer {
constexpr int INSPECT_PORT = 8765;

InspectorWebSocketServer::InspectorWebSocketServer(Inspector* parent)
    : QWebSocketServer("", QWebSocketServer::NonSecureMode, (QObject*)parent),
      m_parent(parent) {
  MZ_COUNT_CTOR(InspectorWebSocketServer);

  logger.debug() << "Creating the inspector websocket server";

  if (!listen(QHostAddress::Any, INSPECT_PORT)) {
    logger.error() << "Failed to listen on port" << INSPECT_PORT;
    return;
  }

  connect(this, &InspectorWebSocketServer::newConnection, this,
          &InspectorWebSocketServer::newConnectionReceived);
  connect(this, &InspectorWebSocketServer::serverError,
          [this](QWebSocketProtocol::CloseCode closeCode) {
            logger.error() << "server error" << closeCode;
          });
  connect(qApp, &QCoreApplication::aboutToQuit, this,
          &InspectorWebSocketServer::close);
}

InspectorWebSocketServer::~InspectorWebSocketServer() {
  MZ_COUNT_DTOR(InspectorWebSocketServer);
}

void InspectorWebSocketServer::newConnectionReceived() {
  QWebSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  QHostAddress address = child->localAddress();

#if !defined(MZ_ANDROID) && !defined(MZ_IOS)
  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  if (address != QHostAddress("::ffff:127.0.0.1") &&
      address != QHostAddress::LocalHost &&
      address != QHostAddress::LocalHostIPv6) {
    logger.warning() << "Accepting connection from localhost only";
    child->close();
    return;
  }
#endif
  logger.info() << "Accepting connection";
  auto transport = new InspectorWebSocketTransport(child);
  m_parent->onConnection(transport);
}

}  // namespace InspectorServer