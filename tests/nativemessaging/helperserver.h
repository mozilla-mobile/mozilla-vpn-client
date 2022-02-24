/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPERSERVER_H
#define HELPERSERVER_H

#include <QThread>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>

class EchoServer final : public QTcpServer {
  Q_OBJECT

 signals:
  void ready();

 public:
  explicit EchoServer(int fuzzy);

  void start();
  void newConnection();

 private:
  const int m_fuzzy;
};

class EchoConnection final : public QObject {
  Q_OBJECT

 public:
  EchoConnection(QTcpSocket* socket, int fuzzy);

 private:
  void maybeStartTimer();

 private:
  QTcpSocket* m_socket = nullptr;
  QTimer m_timer;
  QByteArray m_buffer;
  const int m_fuzzy;
};

class HelperServer final : public QObject {
  Q_OBJECT

 public:
  void start(int fuzzy = 0);
  void stop();

 signals:
  void startServer();
  void ready();

 private:
  EchoServer* m_server = nullptr;
  QThread m_thread;
};

#endif  // HELPERSERVER_H
