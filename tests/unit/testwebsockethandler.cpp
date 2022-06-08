/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.orgß/MPL/2.0/. */

#include "testwebsockethandler.h"
#include "../../src/settingsholder.h"
#include "../../src/constants.h"
#include "../../src/mozillavpn.h"
#include "helper.h"

#include <QtWebSockets/QWebSocketServer>

/**
 * @brief Construct a new Mock Server:: Mock Server object
 *
 * Simple web socket server that exposes signals which tests can spy on.
 */
MockServer::MockServer() {
  m_pWebSocketServer = new QWebSocketServer(
      QStringLiteral("Mock Server"), QWebSocketServer::NonSecureMode, this);
  if (m_pWebSocketServer->listen(QHostAddress::Any, 5000)) {
    connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
            &MockServer::onNewConnection);
  }
}

MockServer::~MockServer() {
  m_pWebSocketServer->close();
  qDeleteAll(m_clients.begin(), m_clients.end());
}

/**
 * @brief Ackowledges a new client has connected to the server.
 */
void MockServer::onNewConnection() {
  QWebSocket* pSocket = m_pWebSocketServer->nextPendingConnection();

  m_clients << pSocket;

  emit newConnection(pSocket->request());

  connect(pSocket, &QWebSocket::disconnected, this,
          &MockServer::onSocketDisconnected);
}

/**
 * @brief Ackowledges a new client has disconnected from the server.
 */
void MockServer::onSocketDisconnected() {
  QWebSocket* pClient = qobject_cast<QWebSocket*>(QObject::sender());
  if (pClient) {
    m_clients.removeAll(pClient);
    pClient->deleteLater();

    emit socketDisconnected();
  }
}

/**
 * @brief Closes each of the open client connection and leaves the server open.
 */
void MockServer::closeEach() {
  QListIterator<QWebSocket*> i(m_clients);
  while (i.hasNext()) {
    QWebSocket* client = i.next();
    client->abort();
  }
}

void TestWebSocketHandler::tst_connectionIsTiedToUserState() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  WebSocketHandler handler;
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Mock start a user log out, this should prompt the connected websocket to
  // disconnect.
  TestHelper::userState = MozillaVPN::UserLoggingOut;
  emit MozillaVPN::instance()->userStateChanged();

  QVERIFY(socketDisconnectedSpy.wait());
  QCOMPARE(socketDisconnectedSpy.count(), 1);

  // Reset the spies.
  newConnectionSpy.clear();
  socketDisconnectedSpy.clear();

  // Mock finish the user log out.
  TestHelper::userState = MozillaVPN::UserNotAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  // Check that the last userStateChange signal did not trigger any connections
  // or disconnections.
  QCOMPARE(newConnectionSpy.count(), 0);
  QCOMPARE(socketDisconnectedSpy.count(), 0);
}

void TestWebSocketHandler::tst_connectionRequestContainsRequiredHeaders() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  WebSocketHandler handler;
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  QList<QVariant> arguments = newConnectionSpy.takeFirst();
  QNetworkRequest requestConnected = arguments.at(0).value<QNetworkRequest>();
  QVERIFY(requestConnected.hasRawHeader("Authorization"));
}

void TestWebSocketHandler::tst_reconnectionAttemptsAfterUnexpectedClose() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));

  WebSocketHandler handler;
  handler.testOverrideRetryInterval(100);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Disconnect client from the server.
  server.closeEach();

  // No need to do anything here.
  //
  // The handler should be polling for reconnection every 100ms,
  // we just wait for the reconnection to actually take place.

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 2);
}

void TestWebSocketHandler::tst_reconnectionAttemptsOnPingTimeout() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  WebSocketHandler handler;
  handler.testOverrideRetryInterval(100);
  // By setting an extremely low ping interval,
  // we are guaranteed to have the ping timer timeout before we get a ping
  // response.
  //
  // That should trigger the server to close the connection and schedule a
  // retry.
  handler.testOverridePingInterval(0);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  // Wait for connection.
  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Once connected we should almost immediatelly disconnect because the ping
  // timer is timed out.
  QVERIFY(socketDisconnectedSpy.wait());
  QCOMPARE(socketDisconnectedSpy.count(), 1);

  // No need to do anything here.
  //
  // The handler should be polling for reconnection every 100ms,
  // we just wait for the reconnection to actually take place.

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 2);

  // Mock a user log out, so that we stop sending so many pings ASAP.
  TestHelper::userState = MozillaVPN::UserLoggingOut;
  emit MozillaVPN::instance()->userStateChanged();
}

static TestWebSocketHandler s_testWebSocketHandler;
