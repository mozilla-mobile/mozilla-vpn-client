/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTBRIDGE_H
#define WEBEXTBRIDGE_H

#include <QByteArray>
#include <QLocalSocket>
#include <QObject>
#include <QTimer>

#include "webextreader.h"

class QIODevice;

class WebExtBridge final : public QObject {
  Q_OBJECT

 public:
  WebExtBridge(const QString& name, QObject* parent = nullptr);

  bool isConnected() const { return m_reader != nullptr; }

  bool sendMessage(const QByteArray& message);

  static int run(int argc, char* argv[]);

 signals:
  void connected();
  void disconnected();
  void messageReceived(const QByteArray& message);

 private slots:
  void stateChanged(QLocalSocket::LocalSocketState socketState);
  void errorOccurred(QLocalSocket::LocalSocketError socketError);

 private:
  void retryConnection();
  void tryPushData();

  QString m_name;
  WebExtReader* m_reader = nullptr;
  QLocalSocket m_socket;
  QByteArray m_buffer;
  QTimer m_retryTimer;
};

#endif  // WEBEXTBRIDGE_H
