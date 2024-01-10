/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorwebsocketconnection.h"

#include <QHostAddress>
#include <QWebSocket>

#include "logging/logger.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("InspectorWebSocketConnection");
}  // namespace

InspectorWebSocketConnection::InspectorWebSocketConnection(
    QObject* parent, QWebSocket* connection)
    : InspectorHandler(parent), m_connection(connection) {
  MZ_COUNT_CTOR(InspectorWebSocketConnection);

#if !defined(MZ_ANDROID) && !defined(MZ_IOS)
  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  Q_ASSERT(connection->localAddress() == QHostAddress("::ffff:127.0.0.1") ||
           connection->localAddress() == QHostAddress::LocalHost ||
           connection->localAddress() == QHostAddress::LocalHostIPv6);
#endif

  logger.debug() << "New connection received";

  Q_ASSERT(m_connection);
  connect(m_connection, &QWebSocket::textMessageReceived, this,
          &InspectorWebSocketConnection::textMessageReceived);
  connect(m_connection, &QWebSocket::binaryMessageReceived, this,
          &InspectorWebSocketConnection::binaryMessageReceived);
}

InspectorWebSocketConnection::~InspectorWebSocketConnection() {
  MZ_COUNT_DTOR(InspectorWebSocketConnection);
  logger.debug() << "Connection released";
}

void InspectorWebSocketConnection::textMessageReceived(const QString& message) {
  logger.debug() << "Text message received";
  recv(message.toLocal8Bit());
}

void InspectorWebSocketConnection::binaryMessageReceived(
    const QByteArray& message) {
  logger.debug() << "Binary message received";
  recv(message);
}

void InspectorWebSocketConnection::send(const QByteArray& buffer) {
  m_connection->sendTextMessage(buffer);
}
