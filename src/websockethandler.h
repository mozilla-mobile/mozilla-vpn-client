/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <QWebSocket>
#include <QTimer>

constexpr const int WEBSOCKET_PING_INTERVAL = 30 * 1000;   // 30s
constexpr const int WEBSOCKET_RETRY_INTERVAL = 30 * 1000;  // 30s

class WebSocketHandler final : public QObject {
  Q_OBJECT
 public:
  WebSocketHandler();
  void initialize();

#ifdef UNIT_TEST
  static void testOverrideWebSocketServerUrl(const QString& url);
  void testOverridePingInterval(int newInterval);
  void testOverrideRetryInterval(int newInterval);
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

  int m_pingInterval = WEBSOCKET_PING_INTERVAL;
  int m_retryInterval = WEBSOCKET_RETRY_INTERVAL;

  static QString s_customWebSocketServerUrl;
};

#endif  // WEBSOCKETHANDLER_H
