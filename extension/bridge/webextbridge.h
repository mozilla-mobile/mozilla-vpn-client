/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTBRIDGE_H
#define WEBEXTBRIDGE_H

#include <QByteArray>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

#include "webextreader.h"

class QIODevice;
 
class WebExtBridge final : public QObject {
  Q_OBJECT
 
 public:
  WebExtBridge(quint16 port, QObject* parent = nullptr);
 
  bool isConnected() const { return m_reader != nullptr; }

  void sendMessage(const QByteArray& message);

 signals:
  void connected();
  void disconnected();
  void messageReceived(const QByteArray& message);
 
 private slots:
  void stateChanged(QAbstractSocket::SocketState socketState);
  void errorOccurred(QAbstractSocket::SocketError socketError);
 
 private:
  void retryConnection();
  void tryPushData();
 
  quint16 m_port;
  WebExtReader* m_reader = nullptr;
  QTcpSocket m_socket;
  QByteArray m_buffer;
  QTimer m_retryTimer;
};

#endif  // WEBEXTBRIDGE_H
 