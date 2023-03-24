/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.orgß/MPL/2.0/. */

#include "testwebsockethandler.h"

#include <QtMath>
#include <QtWebSockets/QWebSocketServer>

#include "app.h"
#include "helper.h"
#include "settingsholder.h"
#include "websocket/websockethandler.h"

/**
 * @brief Construct a new Mock Server:: Mock Server object
 *
 * Simple web socket server that exposes signals which tests can spy on.
 */
MockServer::MockServer() {
  m_pWebSocketServer = new QWebSocketServer(
      QStringLiteral("Mock Server"), QWebSocketServer::NonSecureMode, this);
  open();
}

MockServer::~MockServer() { close(); }

void MockServer::close() {
  m_pWebSocketServer->close();
  disconnect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
             &MockServer::onNewConnection);
  qDeleteAll(m_clients.begin(), m_clients.end());
}

void MockServer::open() {
  if (m_pWebSocketServer->listen(QHostAddress::LocalHost, m_port)) {
    connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
            &MockServer::onNewConnection);
    m_port = m_pWebSocketServer->serverPort();
  } else {
    qWarning() << "Failed to open websockets:"
               << m_pWebSocketServer->errorString();
  }
}

QString MockServer::url() { return QString("ws://localhost:%1").arg(m_port); }

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

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  WebSocketHandler::testOverrideWebSocketServerUrl(server.url());
  WebSocketHandler handler;
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  App::instance()->setUserState(App::UserAuthenticated);

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Mock start a user log out, this should prompt the connected websocket to
  // disconnect.
  App::instance()->setUserState(App::UserLoggingOut);

  QVERIFY(socketDisconnectedSpy.wait());
  QCOMPARE(socketDisconnectedSpy.count(), 1);

  // Reset the spies.
  newConnectionSpy.clear();
  socketDisconnectedSpy.clear();

  // Mock finish the user log out.
  App::instance()->setUserState(App::UserNotAuthenticated);

  // Check that the last userStateChange signal did not trigger any connections
  // or disconnections.
  QCOMPARE(newConnectionSpy.count(), 0);
  QCOMPARE(socketDisconnectedSpy.count(), 0);
}

void TestWebSocketHandler::tst_connectionRequestContainsRequiredHeaders() {
  SettingsHolder settingsHolder;
  settingsHolder.setToken("TOKEN");

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  WebSocketHandler::testOverrideWebSocketServerUrl(server.url());
  WebSocketHandler handler;
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  App::instance()->setUserState(App::UserAuthenticated);

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  QList<QVariant> arguments = newConnectionSpy.takeFirst();
  QNetworkRequest requestConnected = arguments.at(0).value<QNetworkRequest>();
  QVERIFY(requestConnected.hasRawHeader("Authorization"));

  // Let's reset the user state.
  App::instance()->setUserState(App::UserNotAuthenticated);
}

void TestWebSocketHandler::tst_reconnectionAttemptsAfterUnexpectedClose() {
  SettingsHolder settingsHolder;
  settingsHolder.setToken("TOKEN");

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));

  WebSocketHandler::testOverrideWebSocketServerUrl(server.url());
  WebSocketHandler handler;
  handler.testOverrideBaseRetryInterval(100);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  App::instance()->setUserState(App::UserAuthenticated);

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

  // Let's reset the user state.
  App::instance()->setUserState(App::UserNotAuthenticated);
}

void TestWebSocketHandler::
    tst_reconnectionBackoffIsResetOnSuccessfullConnection() {
  SettingsHolder settingsHolder;
  settingsHolder.setToken("TOKEN");

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));

  WebSocketHandler::testOverrideWebSocketServerUrl(server.url());
  WebSocketHandler handler;
  // We don't want too high of a interval here, because intervals are
  // exponentially increasing.
  int testBaseRetryInterval = 5;
  handler.testOverrideBaseRetryInterval(testBaseRetryInterval);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  App::instance()->setUserState(App::UserAuthenticated);

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Close the whole server. All subsequent reconnection attempts will be
  // unsuccesfull.
  server.close();

  // No need to do anything here.
  //
  // The handler should be polling for reconnection,
  // we will let it do that a few times.
  QTest::qWait(500);

  // Reopen the server so reconnections can take place.
  server.open();

  // Before waiting for reconnection, let's see if the interval has increased.
  // It is expected to increase on every reconnection attempt.
  QVERIFY(handler.m_currentBackoffInterval >= testBaseRetryInterval);

  // Wait for reconnection.
  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 2);

  // The reconnection interval should have been reset.
  // When not waiting for reconnection it is set to the special value `0`.
  //
  // We use QTRY here, because it may take a little bit
  // for the `onConnected` handler to be called and the interval to be reset.
  QTRY_COMPARE(handler.m_currentBackoffInterval, 0);

  // Let's reset the user state.
  App::instance()->setUserState(App::UserNotAuthenticated);
}

void TestWebSocketHandler::tst_reconnectionAttemptsOnPingTimeout() {
  SettingsHolder settingsHolder;

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  WebSocketHandler::testOverrideWebSocketServerUrl(server.url());
  WebSocketHandler handler;
  handler.testOverrideBaseRetryInterval(100);
  // By setting an extremely low ping interval,
  // we are guaranteed to have the ping timer timeout before we get a ping
  // response.
  //
  // That should trigger the server to close the connection and schedule a
  // retry.
  handler.testOverridePingInterval(0);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  App::instance()->setUserState(App::UserAuthenticated);

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
  App::instance()->setUserState(App::UserLoggingOut);
}

static TestWebSocketHandler s_testWebSocketHandler;
