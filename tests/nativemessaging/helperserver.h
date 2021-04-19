/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPERSERVER_H
#define HELPERSERVER_H

#include <QThread>
#include <QTcpServer>

class EchoServer : public QTcpServer {
  Q_OBJECT

 signals:
  void ready();

 public:
  void start();
  void newConnection();
};

class HelperServer final : public QObject {
  Q_OBJECT

 public:
  void start();
  void stop();

 signals:
  void startServer();
  void ready();

 private:
  EchoServer* m_server = nullptr;
  QThread m_thread;
};

#endif  // HELPERSERVER_H
