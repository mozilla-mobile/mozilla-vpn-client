/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QByteArray>
#include <QObject>

class QTcpSocket;

class ServerConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerConnection)

 public:
  ServerConnection(QObject* parent, QTcpSocket* connection);
  ~ServerConnection();

 private:
  void readData();
  void writeData(const QByteArray& data);

  void writeState();
  void writeInvalidRequest();

  void processMessage(const QByteArray& message);

 private:
  QTcpSocket* m_connection;

  enum {
    // Reading the length of the body. This step consists in the reading of 4
    // bytes to be read as a uint32_t.
    ReadingLength,

    // Reading the body. The size of the body has been specified during the
    // previous state.
    ReadingBody,
  } m_state = ReadingLength;

  QByteArray m_buffer;
  uint32_t m_messageLength = 0;
};

#endif  // SERVERCONNECTION_H
