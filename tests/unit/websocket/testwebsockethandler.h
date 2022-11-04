/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class QWebSocketServer;

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
  QString url();

 signals:
  void newConnection(QNetworkRequest request);
  void socketDisconnected();

 private slots:
  void onNewConnection();
  void onSocketDisconnected();

 private:
  QWebSocketServer* m_pWebSocketServer;
  QList<QWebSocket*> m_clients;
  quint16 m_port = 0;
};

class TestWebSocketHandler : public TestHelper {
  Q_OBJECT

 private slots:
  void tst_connectionIsTiedToUserState();
  void tst_connectionRequestContainsRequiredHeaders();
  void tst_reconnectionAttemptsAfterUnexpectedClose();
  void tst_reconnectionBackoffIsResetOnSuccessfullConnection();
  void tst_reconnectionAttemptsOnPingTimeout();
};
