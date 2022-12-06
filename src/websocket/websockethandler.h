/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include "exponentialbackoffstrategy.h"

#include <QTimer>
#include <QUrl>
#include <QWebSocket>

constexpr const int WEBSOCKET_PING_INTERVAL_MSEC = 30 * 1000;  // 30s

class ExponentialBackoffStrategy;

class WebSocketHandler final : public QObject {
  Q_OBJECT
 public:
  explicit WebSocketHandler(const QUrl& url = QUrl());
  ~WebSocketHandler();

  void initialize();

#ifdef UNIT_TEST
  void testOverridePingInterval(int newInterval);
  void testOverrideBaseRetryInterval(int newInterval);

  // If currently waiting for until attempting reconnection,
  // this variable will contain the backoff interval.
  //
  // When not waiting, this will be 0.
  int m_currentBackoffInterval = 0;
#endif

 signals:
  void closed();

 private:
  void open();
  void close();
  void sendPing();

  void onUserStateChanged();
  void onConnected();
  void onClose();
  void onMessageReceived(const QString& message);
  void onError(QAbstractSocket::SocketError error);
  void onPong(quint64 elapsedTime);
  void onPingTimeout();

 private:
  QUrl m_url;
  QWebSocket m_webSocket;
  QTimer m_pingTimer;

  int m_pingInterval = WEBSOCKET_PING_INTERVAL_MSEC;
  ExponentialBackoffStrategy m_backoffStrategy;

  bool m_aboutToClose = false;
};

#endif  // WEBSOCKETHANDLER_H
