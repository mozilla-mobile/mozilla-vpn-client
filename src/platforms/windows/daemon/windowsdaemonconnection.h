/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSDAEMONCONNECTION_H
#define WINDOWSDAEMONCONNECTION_H

#include <QObject>

class QLocalSocket;

class WindowsDaemonConnection final : public QObject {
  Q_DISABLE_COPY_MOVE(WindowsDaemonConnection)

 public:
  WindowsDaemonConnection(QObject* parent, QLocalSocket* socket);
  ~WindowsDaemonConnection();

 private:
  void readData();

  void parseCommand(const QByteArray& json);

  void connected();
  void disconnected();

  void write(const QJsonObject& obj);

 private:
  QLocalSocket* m_socket = nullptr;

  QByteArray m_buffer;
};

#endif  // WINDOWSDAEMONCONNECTION_H
