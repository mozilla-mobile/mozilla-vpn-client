/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocket>
#include <QCoreApplication.h>

#include "../inspector.h"
#include "websockettransport.h"


namespace InspectorServer {
constexpr int INSPECT_PORT = 8765;

InspectorWebSocketServer::InspectorWebSocketServer(Inspector* parent)
    : QWebSocketServer("", QWebSocketServer::NonSecureMode, (QObject*)parent),
      m_parent(parent) {

  if (!listen(QHostAddress::Any, INSPECT_PORT)) {
  
    return;
  }

  connect(this, &InspectorWebSocketServer::newConnection, this,
          &InspectorWebSocketServer::newConnectionReceived);
  connect(qApp, &QCoreApplication::aboutToQuit, this,
          &InspectorWebSocketServer::close);
}

InspectorWebSocketServer::~InspectorWebSocketServer() {
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
    child->close();
    return;
  }
#endif
  auto transport = new InspectorWebSocketTransport(child);
  m_parent->onConnection(transport);
}

}  // namespace InspectorServer