/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QByteArray>
#include <QObject>

class QTcpSocket;

namespace WebExtension {

class Connection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Connection)

 public:
  Connection(QObject* parent, QTcpSocket* connection);
  ~Connection();

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

}  // namespace WebExtension
#endif  // SERVERCONNECTION_H
