/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorwebsocketserver.h"

#include <QHostAddress>
#include <QStringView>
#include <QWebSocket>
#include <QWebSocketCorsAuthenticator>
#include <algorithm>
#include <array>

#include "inspectorwebsocketconnection.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("InspectorWebSocketServer");

static constexpr std::array<QStringView, 1> ALLOWED_ORIGINS = {
    u"https://mozilla-mobile.github.io"};

}  // namespace

constexpr int INSPECT_PORT = 8765;

InspectorWebSocketServer::InspectorWebSocketServer(QObject* parent)
    : QWebSocketServer("", QWebSocketServer::NonSecureMode, parent) {
  MZ_COUNT_CTOR(InspectorWebSocketServer);

  logger.debug() << "Creating the inspector websocket server";

  if (!listen(QHostAddress::Any, INSPECT_PORT)) {
    logger.error() << "Failed to listen on port" << INSPECT_PORT;
    return;
  }
  connect(this, &QWebSocketServer::originAuthenticationRequired,
          [](QWebSocketCorsAuthenticator* authMgr) {
            auto ok = std::ranges::any_of(
                ALLOWED_ORIGINS,
                [requestOrigin = authMgr->origin()](QStringView allowedOrigin) {
                  return requestOrigin == allowedOrigin;
                });
            if (!ok) {
              logger.error() << "Rejecting Websocket Connection from: "
                             << authMgr->origin();
            }
            authMgr->setAllowed(ok);
          });

  connect(this, &InspectorWebSocketServer::newConnection, this,
          &InspectorWebSocketServer::newConnectionReceived);
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

  InspectorWebSocketConnection* connection =
      new InspectorWebSocketConnection(this, child);
  connect(child, &QWebSocket::disconnected, connection, &QObject::deleteLater);
}
