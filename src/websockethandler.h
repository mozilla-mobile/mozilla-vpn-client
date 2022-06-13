/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <QWebSocket>
#include <QTimer>

constexpr const int BASE_RETRY_INTERVAL_SEC = 5;               // 5s
constexpr const int MAX_RETRY_INTERVAL_MSEC = 60 * 60 * 1000;  // 1hr
constexpr const int WEBSOCKET_PING_INTERVAL_MSEC = 30 * 1000;  // 30s

class ExponentialBackoffStrategy final : public QObject {
  Q_OBJECT
 public:
  ExponentialBackoffStrategy();
  int scheduleNextAttempt();
  void reset();

#ifdef UNIT_TEST
  void testOverrideBaseRetryInterval(int newInterval);
  void testOverrideMaxRetryInterval(int newInterval);
#endif

 signals:
  void executeNextAttempt();

 private:
  QTimer m_retryTimer;
  int m_retryCounter = 1;
  int m_maxInterval = MAX_RETRY_INTERVAL_MSEC;
  int m_baseInterval = BASE_RETRY_INTERVAL_SEC;
};

class WebSocketHandler final : public QObject {
  Q_OBJECT
 public:
  WebSocketHandler();
  void initialize();

#ifdef UNIT_TEST
  static void testOverrideWebSocketServerUrl(const QString& url);
  void testOverridePingInterval(int newInterval);
  void testOverrideBaseRetryInterval(int newInterval);
#endif

 signals:
  void closed();

 private:
  void open();
  void close();
  void sendPing();
  static QString webSocketServerUrl();
  static bool isUserAuthenticated();

  void onUserStateChanged();
  void onConnected();
  void onClose();
  void onMessageReceived(const QString& message);
  void onError(QAbstractSocket::SocketError error);
  void onPong(quint64 elapsedTime);
  void onPingTimeout();

 private:
  QWebSocket m_webSocket;
  QUrl m_url;
  QTimer m_pingTimer;

  int m_pingInterval = WEBSOCKET_PING_INTERVAL_MSEC;
  ExponentialBackoffStrategy m_backoffStrategy;

  static QString s_customWebSocketServerUrl;
};

#endif  // WEBSOCKETHANDLER_H
