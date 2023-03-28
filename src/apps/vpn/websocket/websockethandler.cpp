/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "websockethandler.h"

#include <QApplication>
#include <QGuiApplication>

#include "app.h"
#include "appconstants.h"
#include "exponentialbackoffstrategy.h"
#include "glean/generated/metrics.h"
#include "gleandeprecated.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "pushmessage.h"
#include "settingsholder.h"
#include "telemetry/gleansample.h"
#include "urlopener.h"

namespace {
Logger logger("WebSocketHandler");
}  // namespace

WebSocketHandler::WebSocketHandler() {
  MZ_COUNT_CTOR(WebSocketHandler);

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
  MZ_COUNT_DTOR(WebSocketHandler);

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
    httpServerUrl = AppConstants::API_PRODUCTION_URL;
  } else {
    httpServerUrl = AppConstants::getStagingServerAddress();
  }

  return httpServerUrl.toLower().replace("http", "ws");
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

  if (m_initialized) {
    logger.debug() << "Attempted to initialize WebSocketHandler, but it has "
                      "already been initialized. Ignoring.";
    return;
  }

  m_initialized = true;

  connect(App::instance(), &App::userStateChanged, this,
          &WebSocketHandler::onUserStateChanged);

#if defined(MZ_ANDROID) || defined(MZ_IOS)
  // From
  // https://developer.apple.com/library/archive/technotes/tn2277/_index.html:
  //
  // "If the system suspends your app and then, later on, reclaims the resources
  // from underneath your listening socket, your app will no longer be listening
  // for connections, even after it has been resumed. The app may or may not be
  // notified of this, depending on how it manages the listening socket. It's
  // generally easier to avoid this problem entirely by closing the listening
  // socket when the app is in the background."
  QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this,
                   [this](Qt::ApplicationState state) {
                     logger.debug()
                         << "ApplicationState change detected" << state;
                     switch (state) {
                       case Qt::ApplicationSuspended:
                         [[fallthrough]];
                       case Qt::ApplicationInactive:
                         close();
                         break;
                       case Qt::ApplicationActive:
                         open();
                         break;
                       default:
                         break;
                     }
                   });
#endif
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
                 << App::instance()->userState();

  if (App::isUserAuthenticated()) {
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
  if (m_webSocket.state() != QAbstractSocket::UnconnectedState &&
      m_webSocket.state() != QAbstractSocket::ClosingState) {
    logger.debug()
        << "Attempted to open a WebSocket connection, but it's already open.";
    return;
  }

  logger.debug() << "Attempting to open WebSocket connection."
                 << webSocketServerUrl();

  mozilla::glean::sample::websocket_connection_attempted.record();
  emit GleanDeprecated::instance()->recordGleanEvent(
      GleanSample::websocketConnectionAttempted);

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

  mozilla::glean::sample::websocket_connected.record();
  emit GleanDeprecated::instance()->recordGleanEvent(
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

  mozilla::glean::sample::websocket_close_attempted.record();
  emit GleanDeprecated::instance()->recordGleanEvent(
      GleanSample::websocketCloseAttempted);
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

  mozilla::glean::sample::websocket_closed.record(
      mozilla::glean::sample::WebsocketClosedExtra{
          ._reason = m_webSocket.closeCode()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::websocketClosed, {{"reason", m_webSocket.closeCode()}});

  m_pingTimer.stop();

  if (App::isUserAuthenticated()) {
#if defined(MZ_ANDROID) || defined(MZ_IOS)
    if (QGuiApplication::applicationState() == Qt::ApplicationSuspended ||
        QGuiApplication::applicationState() == Qt::ApplicationInactive) {
      logger.debug()
          << "Application is suspended. Will not attempt to reopen WebSocket.";
      return;
    }
#endif

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

  mozilla::glean::sample::websocket_pong_timed_out.record();
  emit GleanDeprecated::instance()->recordGleanEvent(
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

  mozilla::glean::sample::websocket_errored.record(
      mozilla::glean::sample::WebsocketErroredExtra{
          ._type = QVariant::fromValue(error).toInt()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
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

  mozilla::glean::sample::push_message_received.record(
      mozilla::glean::sample::PushMessageReceivedExtra{
          ._type = QVariant::fromValue(parsedMessage.type()).toString()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::pushMessageReceived,
      {{"type", QVariant::fromValue(parsedMessage.type()).toString()}});

  parsedMessage.executeAction();
}
