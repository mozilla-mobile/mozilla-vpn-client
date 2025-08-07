/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef Socks5Connection_H
#define Socks5Connection_H

#include "proxyconnection.h"

class QTcpSocket;
class QLocalSocket;

class Socks5Connection final : public ProxyConnection {
  Q_OBJECT

 public:
  explicit Socks5Connection(QTcpSocket* socket);
  explicit Socks5Connection(QLocalSocket* socket);
  ~Socks5Connection() = default;

  enum Socks5State : int {
    ClientGreeting = ProxyState::Handshake,
    AuthenticationMethods,
    ClientConnectionRequest,
    ClientConnectionAddress,
  };

  enum Socks5Replies : uint8_t {
    Success = 0x00u,
    ErrorGeneral = 0x01u,
    ConnectionNotAllowed = 0x02u,
    ErrorNetworkUnreachable = 0x03u,
    ErrorHostUnreachable = 0x04u,
    ErrorConnectionRefused = 0x05u,
    ErrorTTLExpired = 0x06u,
    ErrorCommandNotSupported = 0x07u,
    ErrorAddressNotSupported = 0x08u,
  };

  static Socks5Replies socketErrorToSocks5Rep(
      QAbstractSocket::SocketError error);

  /**
   * @brief Reads <T> from a QIODevice
   * Returns T if enough data
   * was read, std::nothing if nothing was found
   *
   * This function has no side effects on the connection
   * if nothing was returned.
   *
   * There is no formal typecheck that the underlying
   * data of <T> is correct.
   *
   * @tparam T - Type to read
   * @param connection - A QIODevice to read from
   * @return std::optional<T> - The read T if enough bytes were written.
   */
  template <typename T>
  std::optional<T> readPacket();

  // Peek at the socket and determine if this is a socks connection.
  static bool isProxyType(QIODevice* socket);

  void handshakeRead() override;

 private slots:
  void onHostnameResolved(QHostAddress addr);
  void onHostnameNotFound();

 private:
  void setError(Socks5Replies reply, const QString& errorString);
  void configureOutSocket(quint16 port);

  uint8_t m_authNumber = 0;
  uint8_t m_addressType = 0;
};

#endif  // Socks5Connection_H
