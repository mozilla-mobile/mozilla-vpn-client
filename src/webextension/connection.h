/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QByteArray>
#include <QObject>

class QIODevice;

namespace WebExtension {

/**
 * @brief Connection consumes a QIODevice and implements the WebExt Protocol
 * It's a simple Length Prefixed JSON string:
 * 0x00 - 0x03 - A u_int32 setting message message length
 * 0x04 - 0x{Message Length} - A UTF-8 encoded json string. 
 * 
 * It ready on the QIODevice until it's closed or it send's 
 * invalid things. 
 * 
 * It emit's Q_SIGNAL onMessage if a JSON message was recieved
 * 
 */
class Connection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Connection)

 public:
  Connection(QObject* parent, QIODevice* connection);
  ~Connection();

  /**
   * @brief Writes a QJSON object to the device.
   * 
   * @param data - the object :)
   */
  void writeMessage(QJsonObject& data);

  /**
   * @brief - Fired if a new JSON Message was recieved
   */
  Q_SIGNAL void onMessage(QJsonObject& message);

 private:
  void readData();
  void writeData(const QByteArray& data);

  void writeInvalidRequest();

  void processMessage(const QByteArray& message);

 private:
  QIODevice* m_connection;

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
