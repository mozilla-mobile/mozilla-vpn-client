/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SOCKS5CLIENT_H
#define SOCKS5CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Socks5;

class Socks5Client final : public QObject {
  Q_OBJECT

 public:
  Socks5Client(Socks5* parent, QTcpSocket* socket, uint16_t port);
  ~Socks5Client();

 private:
  void readyRead();
  void configureOutSocket();

  /**
   * @brief Copies incoming bytes to another QIODevice
   *
   * @param rx- the source device
   * @param tx- the output device
   * @return qint64 - The Bytes Written, -1 on error.
   */
  static qint64 proxy(QIODevice* rx, QIODevice* tx);

  static uint8_t socketErrorToSocks5Rep(QAbstractSocket::SocketError error);

 private:
  Socks5* m_parent;

  enum {
    ClientGreeting,
    AuthenticationMethods,
    ClientConnectionRequest,
    ClientConnectionAddress,
    Proxy,
  } m_state = ClientGreeting;

  uint8_t m_authNumber = 0;
  QTcpSocket* m_inSocket = nullptr;
  QTcpSocket* m_outSocket = nullptr;

  uint16_t m_socksPort = 0;

  uint8_t m_addressType = 0;
};

#endif  // SOCKS5CLIENT_H
