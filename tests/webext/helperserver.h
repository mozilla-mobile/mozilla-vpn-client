/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPERSERVER_H
#define HELPERSERVER_H

#include <QLocalServer>
#include <QThread>
#include <QTimer>

class QLocalSocket;

class EchoServer final : public QLocalServer {
  Q_OBJECT

 signals:
  void ready();

 public:
  explicit EchoServer(int fuzzy);

  void start(const QString& name);
  void newConnection();

 private:
  const int m_fuzzy;
};

class EchoConnection final : public QObject {
  Q_OBJECT

 public:
  EchoConnection(QLocalSocket* socket, int fuzzy);

 private:
  void maybeStartTimer();

 private:
  QLocalSocket* m_socket = nullptr;
  QTimer m_timer;
  QByteArray m_buffer;
  const int m_fuzzy;
};

class HelperServer final : public QObject {
  Q_OBJECT

 public:
  void start(const QString& name, int fuzzy = 0);
  void stop();

 signals:
  void startServer();
  void ready();

 private:
  EchoServer* m_server = nullptr;
  QString m_name;
  QThread m_thread;
};

#endif  // HELPERSERVER_H
