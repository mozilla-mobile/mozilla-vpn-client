/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class QWebSocketServer;

constexpr const char* MOCK_SERVER_ADDRESS = "ws://localhost:5000";

// Adapted from:
// https://code.woboq.org/qt5/qtwebsockets/tests/auto/websockets/qwebsocket/tst_qwebsocket.cpp.html#EchoServer
class MockServer : public QObject {
  Q_OBJECT

 public:
  explicit MockServer();
  ~MockServer();

  void closeEach();
  void close();
  void open();

 signals:
  void newConnection(QNetworkRequest request);
  void socketDisconnected();

 private slots:
  void onNewConnection();
  void onSocketDisconnected();

 private:
  QWebSocketServer* m_pWebSocketServer;
  QList<QWebSocket*> m_clients;
};

class TestWebSocketHandler : public TestHelper {
  Q_OBJECT

 private slots:
  void tst_connectionIsTiedToUserState();
  void tst_connectionRequestContainsRequiredHeaders();
  void tst_reconnectionAttemptsAfterUnexpectedClose();
  void tst_reconnectionsAreAttemptedUntilSuccessfull();
  void tst_reconnectionBackoffIsResetOnSuccessfullConnection();
  void tst_reconnectionAttemptsOnPingTimeout();
};
