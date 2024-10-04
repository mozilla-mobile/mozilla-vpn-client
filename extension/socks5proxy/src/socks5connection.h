/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef Socks5Connection_H
#define Socks5Connection_H

#include <QByteArray>
#include <QIODevice>
#include <QLocalSocket>
#include <QObject>
#include <QTcpSocket>

class Socks5Connection final : public QObject {
  Q_OBJECT

 public:
  Socks5Connection(QTcpSocket* socket);
  Socks5Connection(QLocalSocket* socket);
  ~Socks5Connection() = default;

  /**
   * @brief Copies incoming bytes to another QIODevice
   *
   * @param rx- the source device
   * @param tx- the output device
   * @return qint64 - The Bytes Written, -1 on error.
   */
  static qint64 proxy(QIODevice* rx, QIODevice* tx);

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
  static std::optional<T> readPacket(QIODevice* connection);

  const QString& clientName() const { return m_clientName; }

 signals:
  void setupOutSocket(QAbstractSocket* socket, const QHostAddress& dest);
  void dataSentReceived(qint64 sent, qint64 received);

 private:
  void configureOutSocket(const QHostAddress& dest, quint16 port);
  void dnsResolutionFinished(quint16 port);
  void readyRead();

  enum {
    ClientGreeting,
    AuthenticationMethods,
    ClientConnectionRequest,
    ClientConnectionAddress,
    Proxy,
  } m_state = ClientGreeting;

  uint8_t m_authNumber = 0;
  QIODevice* m_inSocket = nullptr;
  QTcpSocket* m_outSocket = nullptr;

  QString m_clientName;
  uint16_t m_socksPort = 0;

  uint8_t m_addressType = 0;

  int m_dnsLookupAttempts = 0;
};

#endif  // Socks5Connection_H
