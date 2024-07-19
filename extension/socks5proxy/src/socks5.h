/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SOCKS5_H
#define SOCKS5_H

#include <QObject>
#include <QTcpServer>

class QAbstractSocket;
class QHostAddress;

class Socks5 final : public QObject {
  Q_OBJECT
  Q_PROPERTY(uint16_t connections READ connections NOTIFY connectionsChanged);

 public:
  explicit Socks5(uint16_t port, QHostAddress listenAddress, QObject* parent);
  ~Socks5();

  void clientDismissed();

  int port() const;

  uint16_t connections() const { return m_clientCount; }

 signals:
  void connectionsChanged();
  void incomingConnection(QAbstractSocket* socket, const QHostAddress& peer);
  void outgoingConnection(QAbstractSocket* socket, const QHostAddress& dest);
  void dataSentReceived(qint64 sent, qint64 received);

 private:
  void newConnection();

 private:
  QTcpServer m_server;
  uint16_t m_clientCount = 0;
  bool m_shuttingDown = false;
};

#endif  // SOCKS5_H
