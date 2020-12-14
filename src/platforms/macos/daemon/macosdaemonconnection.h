/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDAEMONCONNECTION_H
#define MACOSDAEMONCONNECTION_H

#include <QObject>

class QLocalSocket;

class MacOSDaemonConnection final : public QObject {
  Q_DISABLE_COPY_MOVE(MacOSDaemonConnection)

 public:
  MacOSDaemonConnection(QObject* parent, QLocalSocket* socket);
  ~MacOSDaemonConnection();

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

#endif  // MACOSDAEMONCONNECTION_H
