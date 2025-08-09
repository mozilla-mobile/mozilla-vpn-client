/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5connection.h"

#include "dnsresolver.h"
#include "socks5.h"

#ifdef Q_OS_WIN
#  include <winsock2.h>
#  include <ws2ipdef.h>

#  include "winutils.h"
#else
#  include <arpa/inet.h>
#endif

#include <QDnsLookup>
#include <QHostAddress>
#include <QIODevice>
#include <QLocalSocket>
#include <QTcpSocket>

namespace {

#ifdef Q_OS_WIN
#  define PACK(__Declaration__) \
    __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#else
#  define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

PACK(struct ClientGreetingPacket {
  uint8_t version;
  uint8_t nauth;
});

PACK(struct ServerChoicePacket {
  uint8_t version;
  uint8_t cauth;
});

PACK(struct ClientConnectionRequestPacket {
  uint8_t version;
  uint8_t cmd;
  uint8_t rsv;
  uint8_t atype;
});

PACK(struct ClientConnectionAddressIpv4Packet {
  uint8_t ip_dst[4];
  uint16_t port;
});

PACK(struct ClientConnectionAddressDomainNamePacket {
  uint8_t cmd;
  uint16_t port;
});

PACK(struct ClientConnectionAddressIpv6Packet {
  uint8_t ip_dst[16];
  uint16_t port;
});

PACK(struct ServerResponsePacket {
  uint8_t m_version = 0x05;
  uint8_t m_rep = 0x01;
  uint8_t m_rsv = 0x00;
  uint8_t m_atype = 0x01;
  uint32_t m_ip_dst = 0x00;
  uint16_t m_port = 0x00;
});

ServerResponsePacket createServerResponsePacket(uint8_t rep,
                                                uint16_t port = 0x00u) {
  return ServerResponsePacket{0x05, rep, 0x00, 0x01, 0x00, port};
}

}  // namespace

void Socks5Connection::setError(Socks5Replies reason,
                                const QString& errorString) {
  // A Server response message is only sent in certain states.
  switch (m_state) {
    case ClientGreeting:
      [[fallthrough]];
    case ClientConnectionRequest:
      [[fallthrough]];
    case ClientConnectionAddress: {
      ServerResponsePacket packet(createServerResponsePacket(reason));
      m_clientSocket->write((char*)&packet, sizeof(ServerResponsePacket));
      break;
    }

    default:
      break;
  }

  m_errorString = errorString;
  setState(Closed);
}

void Socks5Connection::handshakeRead() {
  switch (m_state) {
    case ClientGreeting: {
      const auto packet = readPacket<ClientGreetingPacket>();
      if (!packet) {
        return;
      }
      uint8_t version = packet.value().version;
      if (version != 0x5) {
        // We only currently want to support socks5.
        // as otherwise we could not support udp or auth.
        auto msg = QString("SOCKS version %1 not supported").arg(version);
        setError(ErrorGeneral, msg);
        return;
      }
      m_authNumber = packet.value().nauth;
      setState(AuthenticationMethods);
      [[fallthrough]];
    }

    case AuthenticationMethods: {
      if (m_clientSocket->bytesAvailable() < (qint64)m_authNumber) {
        return;
      }

      char buffer[INT8_MAX];
      if (m_clientSocket->read(buffer, m_authNumber) != m_authNumber) {
        setError(ErrorGeneral, m_clientSocket->errorString());
        return;
      }

      ServerChoicePacket packet;
      packet.version = 0x5;
      packet.cauth = 0x00;  // TODO: authentication check!

      if (m_clientSocket->write((const char*)&packet, sizeof(ServerChoicePacket)) !=
          sizeof(ServerChoicePacket)) {
        setError(ErrorGeneral, m_clientSocket->errorString());
        return;
      }

      setState(ClientConnectionRequest);
      [[fallthrough]];
    }

    case ClientConnectionRequest: {
      auto const packet = readPacket<ClientConnectionRequestPacket>();
      if (!packet) {
        return;
      }
      if (packet.value().version != 0x5 || packet.value().rsv != 0x00) {
        setError(ErrorGeneral, "Malformed connection request");
        return;
      }
      if (packet.value().cmd != 0x01u /* connection */) {
        setError(ErrorCommandNotSupported, "Command not supported");
        return;
      }
      m_addressType = packet.value().atype;
      setState(ClientConnectionAddress);
      [[fallthrough]];
    }

    case ClientConnectionAddress: {
      if (m_addressType == 0x01 /* Ipv4 */) {
        auto const packet = readPacket<ClientConnectionAddressIpv4Packet>();
        if (!packet) {
          return;
        }
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = ntohs(packet.value().port);
        memcpy(&(sa.sin_addr), (void*)&packet.value().ip_dst, 4);
        m_destAddress.setAddress((struct sockaddr*)&sa);

        configureOutSocket(sa.sin_port);
      }

      else if (m_addressType == 0x03 /* Domain name */) {
        if (m_clientSocket->bytesAvailable() == 0) return;

        uint8_t length;
        if (m_clientSocket->read((char*)&length, 1) != 1) {
          setError(ErrorGeneral, m_clientSocket->errorString());
          return;
        }

        char buffer[UINT8_MAX];
        if (m_clientSocket->read(buffer, length) != length) {
          setError(ErrorGeneral, m_clientSocket->errorString());
          return;
        }

        uint16_t port;
        if (m_clientSocket->read((char*)&port, sizeof(port)) != sizeof(port)) {
          setError(ErrorGeneral, m_clientSocket->errorString());
          return;
        }

        m_destHostname = QString::fromUtf8(buffer, length);
        m_destPort = htons(port);
        setState(Resolve);

        DNSResolver::instance()->resolveAsync(m_destHostname, this);
      }

      else if (m_addressType == 0x04 /* Ipv6 */) {
        auto const packet = readPacket<ClientConnectionAddressIpv6Packet>();
        if (!packet) {
          return;
        }
        struct sockaddr_in6 sa;
        sa.sin6_family = AF_INET6;
        sa.sin6_port = ntohs(packet.value().port);
        memcpy(&(sa.sin6_addr), (void*)&packet.value().ip_dst, 16);
        m_destAddress.setAddress((struct sockaddr*)&sa);

        configureOutSocket(sa.sin6_port);
      }

      else {
        setError(ErrorAddressNotSupported, "Address type not supported");
        return;
      }
    }

    break;

    default:
      Q_ASSERT(false);
      break;
  }
}

void Socks5Connection::onHostnameResolved(QHostAddress resolved) {
  if (m_destSocket != nullptr) {
    // We might get multiple ip results.
    return;
  }
  m_destAddress = resolved;
  Q_ASSERT(!resolved.isNull());
  configureOutSocket(m_destPort);
}

void Socks5Connection::configureOutSocket(quint16 port) {
  Q_ASSERT(!m_destAddress.isNull());

  m_destSocket = createDestSocket<QTcpSocket>(m_destAddress, port);
  if (!m_destSocket) {
    setError(ErrorGeneral, m_errorString);
    return;
  }

  connect(m_destSocket, &QTcpSocket::connected, this, [this]() {
    ServerResponsePacket packet(createServerResponsePacket(0x00, m_clientPort));
    if (m_clientSocket->write((char*)&packet, sizeof(ServerResponsePacket)) !=
        sizeof(ServerResponsePacket)) {
      setError(ErrorGeneral, m_clientSocket->errorString());
      return;
    }

    setState(Proxy);
  });

  connect(m_destSocket, &QTcpSocket::disconnected, this,
          [this]() { setState(Closed); });

  connect(m_destSocket, &QTcpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError error) {
            if (error == QAbstractSocket::RemoteHostClosedError) {
              setState(Closed);
            } else {
              setError(ErrorGeneral, m_destSocket->errorString());
            }
          });

  m_destSocket->connectToHost(m_destAddress, port);
}

void Socks5Connection::onHostnameNotFound() {
  setError(ErrorHostUnreachable, "Failed to Resolve DNS Query");
}

Socks5Connection::Socks5Replies Socks5Connection::socketErrorToSocks5Rep(
    QAbstractSocket::SocketError error) {
  switch (error) {
    case QAbstractSocket::HostNotFoundError:
      return ErrorHostUnreachable;
    case QAbstractSocket::ConnectionRefusedError:
      return ErrorConnectionRefused;
    case QAbstractSocket::NetworkError:
      return ErrorNetworkUnreachable;
    case QAbstractSocket::SocketTimeoutError:
      return ErrorTTLExpired;
    default:
      return ErrorGeneral;
  }
}

template <typename T>
std::optional<T> Socks5Connection::readPacket() {
  // There are not enough bytes to read don't touch the connection.
  if (m_clientSocket->bytesAvailable() < (qint64)sizeof(T)) {
    return {};
  }
  m_clientSocket->startTransaction();
  T packet;
  if (m_clientSocket->read((char*)&packet, sizeof(T)) != sizeof(T)) {
    // If we did not read the correct amount of data
    // Abort the transaction and reset the buffer, so the
    // caller can try to read again.
    m_clientSocket->rollbackTransaction();
    return {};
  }
  m_clientSocket->commitTransaction();
  return packet;
}

// Peek at the socket and determine if this is a socks connection.
bool Socks5Connection::isProxyType(QIODevice* socket) {
  QByteArray data = socket->peek(sizeof(ClientGreetingPacket));
  if (data.length() < sizeof(ClientGreetingPacket)) {
    return false;
  }
  // This is probably a SOCKS proxy connection if the first byte indicates version 4 or 5.
  return (data.at(0) == 0x05) || (data.at(0) == 0x04);
}
