/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.orgß/MPL/2.0/. */

#include "testwebsockethandler.h"
#include "../../src/settingsholder.h"
#include "../../src/constants.h"
#include "../../src/mozillavpn.h"
#include "helper.h"

#include <QtWebSockets/QWebSocketServer>
#include <QtMath>

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
  if (m_pWebSocketServer->listen(QHostAddress::Any, 5000)) {
    connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
            &MockServer::onNewConnection);
  }
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
  handler.testOverrideBaseRetryInterval(100);
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

void TestWebSocketHandler::tst_reconnectionsAreAttemptedUntilSuccessfull() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));

  WebSocketHandler handler;
  // We don't want too high of a interval here, because intervals are
  // exponentially increasing.
  handler.testOverrideBaseRetryInterval(5);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Close the whole server. All subsequent reconnection attempts will be
  // unsuccesfull.
  server.close();

  // No need to do anything here.
  //
  // The handler should be polling for reconnection every 5ms,
  // we will let it do that a few times.
  QTest::qWait(100);

  // Reopen the server so reconnections can take place.
  server.open();

  // Wait for reconnection.
  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 2);
}

void TestWebSocketHandler::tst_reconnectionBackoffTimeExponentiallyIncreases() {
  ExponentialBackoffStrategy backoffStrategy;

  int testBaseRetryInterval = 5;
  int testMaxRetries = 3;
  backoffStrategy.testOverrideBaseRetryInterval(testBaseRetryInterval);
  backoffStrategy.testOverrideMaxRetryInterval(
      qPow(testBaseRetryInterval, testMaxRetries));

  int callCount = 0;
  connect(&backoffStrategy, &ExponentialBackoffStrategy::executeNextAttempt,
          [&callCount]() { callCount++; });

  for (int i = 0; i < testMaxRetries; i++) {
    // Schedule an attempt.
    int nextAttemptIn = backoffStrategy.scheduleNextAttempt();
    // Verify interval is the expected value.
    QCOMPARE(nextAttemptIn, qPow(testBaseRetryInterval, i + 1));
    // `testFn` should only have been scheduled at this point, not called.
    QCOMPARE(callCount, i);
    // Wait for testFn to be executed.
    QVERIFY(
        QTest::qWaitFor([&callCount, i]() { return callCount == (i + 1); }));
  }

  // Inside the loop we have reached max retries, so we expect the interval to
  // be the same as the last one now.

  // Schedule an attempt.
  int nextAttemptIn = backoffStrategy.scheduleNextAttempt();
  // Verify interval is the expected value.
  QCOMPARE(nextAttemptIn, qPow(testBaseRetryInterval, testMaxRetries));
  // `testFn` should only have been scheduled at this point, not called.
  QCOMPARE(callCount, testMaxRetries);
  // Wait for testFn to be executed.
  QVERIFY(QTest::qWaitFor([&callCount, testMaxRetries]() {
    return callCount == testMaxRetries + 1;
  }));

  // After a reset, the interval should be back to base interval.
  backoffStrategy.reset();

  // Schedule an attempt.
  nextAttemptIn = backoffStrategy.scheduleNextAttempt();
  // Verify interval is the expected value.
  QCOMPARE(nextAttemptIn, testBaseRetryInterval);
  // `testFn` should only have been scheduled at this point, not called.
  QCOMPARE(callCount, testMaxRetries + 1);
  // Wait for testFn to be executed.
  QVERIFY(QTest::qWaitFor([&callCount, testMaxRetries]() {
    return callCount == testMaxRetries + 2;
  }));
}

void TestWebSocketHandler::
    tst_reconnectionBackoffIsResetOnSuccessfullConnection() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

  int testBaseRetryInterval = 5;
  WebSocketHandler handler;
  handler.testOverrideBaseRetryInterval(testBaseRetryInterval);
  handler.initialize();

  // Mock a user log in, this should prompt a new websocket connection.
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();

  QVERIFY(newConnectionSpy.wait());
  QCOMPARE(newConnectionSpy.count(), 1);

  // Close the whole server. All subsequent reconnection attempts will be
  // unsuccesfull.
  server.close();

  // No need to do anything here.
  //
  // The handler should be polling for reconnection every 5ms,
  // we will let it do that at least 3 times.
  QTest::qWait(qPow(testBaseRetryInterval, 5));

  // Start a timer
  QElapsedTimer timer;
  timer.start();

  // Reopen the server so reconnections can take place.
  server.open();

  // Wait for reconnection.
  QVERIFY(newConnectionSpy.wait());

  // We have to be very loose here with the checks.
  //
  // We know there were at least three retries,
  // the new connection should have taken at least more than the third
  // interval.
  QVERIFY(timer.elapsed() > qPow(testBaseRetryInterval, 3));
  QCOMPARE(newConnectionSpy.count(), 2);

  // Give it just a bit of time for the `onConnected` handler to be called.
  // The new connection spy is triggered before that and if we don't wait
  // there is no time for the handler to reset the interval.
  //
  // Yes, this is a bit hacky.
  QTest::qWait(50);

  // Close the open connections to prompt a new reconnection.
  server.closeEach();
  timer.restart();

  // Wait for reconnection.
  QVERIFY(newConnectionSpy.wait());
  // Again, we have to be loose with these cheks and checking if this
  // took less than just one interval is asking for too much precision of
  // QElapsedTimer.
  //
  // Backoff interval should have been reset, if this interval is smaller than
  // the previous we should be fine.
  QVERIFY(timer.elapsed() < qPow(testBaseRetryInterval, 2));
}

void TestWebSocketHandler::tst_reconnectionAttemptsOnPingTimeout() {
  SettingsHolder settingsHolder;
  settingsHolder.setFeaturesFlippedOn(QStringList{"websocket"});
  WebSocketHandler::testOverrideWebSocketServerUrl(MOCK_SERVER_ADDRESS);

  MockServer server;
  QSignalSpy newConnectionSpy(&server, SIGNAL(newConnection(QNetworkRequest)));
  QSignalSpy socketDisconnectedSpy(&server, SIGNAL(socketDisconnected()));

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
