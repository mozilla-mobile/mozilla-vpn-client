/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "websockethandler.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "urlopener.h"
#include "exponentialbackoffstrategy.h"
#include "pushmessage.h"
#include "telemetry/gleansample.h"

namespace {
Logger logger(LOG_MAIN, "WebSocketHandler");
}  // namespace

WebSocketHandler::WebSocketHandler() {
  MVPN_COUNT_CTOR(WebSocketHandler);

  connect(&m_webSocket, &QWebSocket::connected, this,
          &WebSocketHandler::onConnected);
  connect(&m_webSocket, &QWebSocket::disconnected, this,
          &WebSocketHandler::onClose);
  connect(&m_webSocket,
          QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
          &WebSocketHandler::onError);
  connect(&m_webSocket, &QWebSocket::pong, this, &WebSocketHandler::onPong);

  connect(&m_pingTimer, &QTimer::timeout, this,
          &WebSocketHandler::onPingTimeout);
  m_pingTimer.setSingleShot(true);

  connect(&m_backoffStrategy, &ExponentialBackoffStrategy::executeNextAttempt,
          this, &WebSocketHandler::open);
}

WebSocketHandler::~WebSocketHandler() {
  MVPN_COUNT_DTOR(WebSocketHandler);

  m_webSocket.disconnect();
  m_backoffStrategy.disconnect();
  m_pingTimer.disconnect();
}

// static
QString WebSocketHandler::s_customWebSocketServerUrl = "";

// static
QString WebSocketHandler::webSocketServerUrl() {
  if (!s_customWebSocketServerUrl.isEmpty()) {
    return s_customWebSocketServerUrl;
  }

  QString httpServerUrl;
  if (Constants::inProduction()) {
    httpServerUrl = Constants::API_PRODUCTION_URL;
  } else {
    httpServerUrl = Constants::getStagingServerAddress();
  }

  return httpServerUrl.toLower().replace("http", "ws");
}

// static
bool WebSocketHandler::isUserAuthenticated() {
  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  return vpn->userState() == MozillaVPN::UserAuthenticated;
}

#ifdef UNIT_TEST
// static
void WebSocketHandler::testOverrideWebSocketServerUrl(const QString& url) {
  WebSocketHandler::s_customWebSocketServerUrl = url;
}

void WebSocketHandler::testOverridePingInterval(int newInterval) {
  m_pingInterval = newInterval;
}

void WebSocketHandler::testOverrideBaseRetryInterval(int newInterval) {
  m_backoffStrategy.testOverrideBaseRetryInterval(newInterval);
}
#endif

void WebSocketHandler::initialize() {
  logger.debug() << "Initialize";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  connect(vpn, &MozillaVPN::userStateChanged, this,
          &WebSocketHandler::onUserStateChanged);
}

/**
 * @brief Acknowledges the user state has changed.
 *
 * When the user has authenticated, a WebSocket connection is openened.
 * Otherwise it's closed.
 *
 * The Guardian WebSocket does not accept unauthenticated users. Attempting to
 * connect while unauthenticated would result is an infinite retry loop.
 */
void WebSocketHandler::onUserStateChanged() {
  logger.debug() << "User state change detected:"
                 << MozillaVPN::instance()->userState();

  if (isUserAuthenticated()) {
    open();
  } else {
    close();
  }
}

/**
 * @brief Opens the websocket connection.
 *
 * No-op in case the connection is already open.
 */
void WebSocketHandler::open() {
  emit MozillaVPN::instance()->recordGleanEvent(
      GleanSample::websocketConnectionAttempted);

  if (m_webSocket.state() != QAbstractSocket::UnconnectedState &&
      m_webSocket.state() != QAbstractSocket::ClosingState) {
    logger.debug()
        << "Attempted to open a WebSocket connection, but it's already open.";
    return;
  }

  logger.debug() << "Attempting to open WebSocket connection."
                 << webSocketServerUrl();

  QNetworkRequest request;
  request.setRawHeader("Authorization",
                       SettingsHolder::instance()->token().toLocal8Bit());
  request.setUrl(QUrl(webSocketServerUrl()));
  m_webSocket.open(request);
}

/**
 * @brief Ackowledges the WebSocket has been succesfully connected.
 */
void WebSocketHandler::onConnected() {
  logger.debug() << "WebSocket connected";

  m_aboutToClose = false;

  emit MozillaVPN::instance()->recordGleanEvent(
      GleanSample::websocketConnected);

  m_backoffStrategy.reset();
#ifdef UNIT_TEST
  m_currentBackoffInterval = 0;
#endif

  connect(&m_webSocket, &QWebSocket::textMessageReceived, this,
          &WebSocketHandler::onMessageReceived);

  sendPing();
}

/**
 * @brief Closes the websocket connection.
 *
 * No-op in case the connection is already closed.
 */
void WebSocketHandler::close() {
  emit MozillaVPN::instance()->recordGleanEvent(
      GleanSample::websocketCloseAttempted);

  // QAbstractSocket may throw a write error when attempting to close the
  // underlying socket (see:
  // https://code.woboq.org/qt5/qtwebsockets/src/websockets/qwebsocket_p.cpp.html#357).
  //
  // The error then triggers the onError handler, which attempts to close again
  // causing an infinite loop.
  if (m_aboutToClose) {
    logger.debug() << "Attempted to close a WebSocket connection, but it's "
                      "already closing/closed.";
    return;
  }

  logger.debug() << "Closing WebSocket";
  m_aboutToClose = true;
  m_webSocket.close();
}

/**
 * @brief Ackowledges the WebSocket has been closed.
 *
 * May trigger a reconnection attempt depending on whether the user is
 * authenticated.
 */
void WebSocketHandler::onClose() {
  // https://doc.qt.io/qt-6/qwebsocketprotocol.html#CloseCode-enum
  logger.debug() << "WebSocket closed:" << m_webSocket.closeCode();

  m_aboutToClose = false;

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::websocketClosed, {{"reason", m_webSocket.closeCode()}});

  m_pingTimer.stop();

  if (isUserAuthenticated()) {
    int nextAttemptIn = m_backoffStrategy.scheduleNextAttempt();
    logger.debug()
        << "User is authenticated. Will attempt to reopen websocket in:"
        << nextAttemptIn;

#ifdef UNIT_TEST
    m_currentBackoffInterval = nextAttemptIn;
#endif
  } else {
    logger.debug()
        << "User is not authenticated. Will not attempt to reopen WebSocket.";
  }
}

/**
 * @brief Sends a ping to the WebSocket server.
 *
 * WebSocket connections may silently be broken. Periodically pinging the server
 * keeps track of the connection. Everytime a new ping is sent, a timer is
 * started. If a response is received before the timer is up, the connection is
 * alive and a new ping is scheduled. Otherwise the WebSocket is closed a a
 * reconnection attempt is scheduled.
 */
void WebSocketHandler::sendPing() {
  m_webSocket.ping();
  m_pingTimer.start(m_pingInterval);
}

/**
 * @brief Acknowledges a pong response from the WebSocket server. Schedules the
 * next ping.
 *
 * @param elapsedTime How long it took to get the response since a ping was sent
 * out.
 */
void WebSocketHandler::onPong(quint64 elapsedTime) {
  logger.debug() << "WebSocket pong" << elapsedTime;

  m_pingTimer.stop();
  QTimer::singleShot(m_pingInterval, this, &WebSocketHandler::sendPing);
}

/**
 * @brief Handles the timeout of the ping timer.
 *
 * Reaching this timeout means connection with the server was silently broken
 * e.g. due to the network being disconnected. The connection will be explicitly
 * closed and a reconnection attempt scheduled.
 */
void WebSocketHandler::onPingTimeout() {
  logger.debug() << "Timed out waiting for ping response";

  emit MozillaVPN::instance()->recordGleanEvent(
      GleanSample::websocketPongTimedOut);

  close();
}

/**
 * @brief Ackowledges there was a WebSocket error.
 *
 * Closes the websocket, is the user is authenticated a new connection attempt
 * will be triggered after WEBSOCKET_RETRY_INTERVAL_MSEC.
 */
void WebSocketHandler::onError(QAbstractSocket::SocketError error) {
  logger.debug() << "WebSocket error:" << error;

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::websocketErrored,
      {{"type", QVariant::fromValue(error).toInt()}});

  close();
}

/**
 * @brief Acknowledges a message was received from the WebSocket server.
 *
 * Each message will trigger a different task.
 */
void WebSocketHandler::onMessageReceived(const QString& message) {
  logger.debug() << "Message received:" << message;

  PushMessage parsedMessage(message);
  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::pushMessageReceived,
      {{"type", QVariant::fromValue(parsedMessage.type()).toString()}});

  parsedMessage.executeAction();
}
