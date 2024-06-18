/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5client.h"

#include "socks5.h"

#ifdef Q_OS_WIN
#  include <winsock2.h>
#  include <ws2ipdef.h>
#else
#  include <arpa/inet.h>
#endif

#include <QHostAddress>

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

ServerResponsePacket createServerResponsePacket(uint8_t rep, uint16_t port) {
  return ServerResponsePacket{0x05, rep, 0x00, 0x01, 0x00, port};
}

}  // namespace

Socks5Client::Socks5Client(Socks5* parent, QTcpSocket* socket, uint16_t port)
    : QObject(parent), m_parent(parent), m_inSocket(socket), m_socksPort(port) {
  connect(m_inSocket, &QTcpSocket::disconnected, this, &QObject::deleteLater);
  connect(m_inSocket, &QIODevice::readyRead, this, &Socks5Client::readyRead);

  readyRead();
}

Socks5Client::~Socks5Client() { m_parent->clientDismissed(); }

void Socks5Client::readyRead() {
  switch (m_state) {
    case ClientGreeting: {
      if (m_inSocket->bytesAvailable() < (qint64)sizeof(ClientGreetingPacket)) {
        return;
      }

      ClientGreetingPacket packet;
      if (m_inSocket->read((char*)&packet, sizeof(ClientGreetingPacket)) !=
          sizeof(ClientGreetingPacket)) {
        m_inSocket->close();
        return;
      }

      if (packet.version != 0x5) {
        ServerResponsePacket packet(createServerResponsePacket(
            0x01 /*  general SOCKS server failure */, 0x00));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }

      m_state = AuthenticationMethods;
      m_authNumber = packet.nauth;
      [[fallthrough]];
    }

    case AuthenticationMethods: {
      if (m_inSocket->bytesAvailable() < (qint64)m_authNumber) {
        return;
      }

      char buffer[INT8_MAX];
      if (m_inSocket->read(buffer, m_authNumber) != m_authNumber) {
        m_inSocket->close();
        return;
      }

      ServerChoicePacket packet;
      packet.version = 0x5;
      packet.cauth = 0x00;  // TODO: authentication check!

      if (m_inSocket->write((const char*)&packet, sizeof(ServerChoicePacket)) !=
          sizeof(ServerChoicePacket)) {
        m_inSocket->close();
        return;
      }

      m_state = ClientConnectionRequest;
      [[fallthrough]];
    }

    case ClientConnectionRequest: {
      if (m_inSocket->bytesAvailable() <
          (qint64)sizeof(ClientConnectionRequestPacket)) {
        return;
      }

      ClientConnectionRequestPacket packet;
      if (m_inSocket->read((char*)&packet,
                           sizeof(ClientConnectionRequestPacket)) !=
          sizeof(ClientConnectionRequestPacket)) {
        m_inSocket->close();
        return;
      }

      if (packet.version != 0x5 || packet.rsv != 0x00) {
        ServerResponsePacket packet(createServerResponsePacket(
            0x01 /*  general SOCKS server failure */, 0x00));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }

      if (packet.cmd != 0x01 /* connection */) {
        ServerResponsePacket packet(
            createServerResponsePacket(0x07 /* Command not supported */, 0x00));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }

      m_state = ClientConnectionAddress;
      m_addressType = packet.atype;
      [[fallthrough]];
    }

    case ClientConnectionAddress: {
      if (m_addressType == 0x01 /* Ipv4 */) {
        if (m_inSocket->bytesAvailable() <
            (qint64)sizeof(ClientConnectionAddressIpv4Packet)) {
          return;
        }

        ClientConnectionAddressIpv4Packet packet;
        if (m_inSocket->read((char*)&packet,
                             sizeof(ClientConnectionAddressIpv4Packet)) !=
            sizeof(ClientConnectionAddressIpv4Packet)) {
          m_inSocket->close();
          return;
        }

        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = ntohs(packet.port);
        memcpy(&(sa.sin_addr), (void*)&packet.ip_dst, 4);

        m_outSocket = new QTcpSocket(this);
        m_outSocket->connectToHost(QHostAddress((struct sockaddr*)&sa),
                                   sa.sin_port);
        configureOutSocket();
      }

      else if (m_addressType == 0x03 /* Domain name */) {
        if (m_inSocket->bytesAvailable() == 0) return;

        uint8_t length;
        if (m_inSocket->read((char*)&length, 1) != 1) {
          m_inSocket->close();
          return;
        }

        char buffer[INT8_MAX];
        if (m_inSocket->read(buffer, length) != length) {
          m_inSocket->close();
          return;
        }

        uint16_t port;
        if (m_inSocket->read((char*)&port, sizeof(port)) != sizeof(port)) {
          m_inSocket->close();
          return;
        }

        m_outSocket = new QTcpSocket(this);
        m_outSocket->connectToHost(QByteArray(buffer, length), ntohs(port));
        configureOutSocket();
      }

      else if (m_addressType == 0x04 /* Ipv6 */) {
        if (m_inSocket->bytesAvailable() <
            (qint64)sizeof(ClientConnectionAddressIpv6Packet)) {
          return;
        }

        ClientConnectionAddressIpv6Packet packet;
        if (m_inSocket->read((char*)&packet,
                             sizeof(ClientConnectionAddressIpv6Packet)) !=
            sizeof(ClientConnectionAddressIpv6Packet)) {
          m_inSocket->close();
          return;
        }

        struct sockaddr_in6 sa;
        sa.sin6_family = AF_INET6;
        sa.sin6_port = ntohs(packet.port);
        memcpy(&(sa.sin6_addr), (void*)&packet.ip_dst, 16);

        m_outSocket = new QTcpSocket(this);
        m_outSocket->connectToHost(QHostAddress((struct sockaddr*)&sa),
                                   sa.sin6_port);
        configureOutSocket();
      }

      else {
        ServerResponsePacket packet(createServerResponsePacket(
            0x08 /* Address type not supported */, 0x00));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }
    }

    break;

    case Proxy: {
      qint64 bytes = proxy(m_inSocket, m_outSocket);
      if (bytes) {
        emit m_parent->dataSentReceived(bytes, 0);
      }
    } break;

    default:
      Q_ASSERT(false);
      break;
  }
}

qint64 Socks5Client::proxy(QIODevice* a, QIODevice* b) {
  Q_ASSERT(a && b);

  qint64 bytes = 0;
  char buffer[4096];
  while (a->bytesAvailable()) {
    qint64 val =
        a->read(buffer, qMin(a->bytesAvailable(), (qint64)sizeof(buffer)));
    if (val <= 0 || b->write(buffer, val) != val) {
      return -1;
    }

    bytes += val;
  }

  return bytes;
}

void Socks5Client::configureOutSocket() {
  connect(m_outSocket, &QTcpSocket::connected, this, [this]() {
    m_state = Proxy;

    ServerResponsePacket packet(createServerResponsePacket(0x00, m_socksPort));
    if (m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket)) !=
        sizeof(ServerResponsePacket)) {
      m_inSocket->close();
      return;
    }

    readyRead();
  });

  connect(m_outSocket, &QTcpSocket::readyRead, this, [this]() {
    qint64 bytes = proxy(m_outSocket, m_inSocket);
    if (bytes > 0) {
      emit m_parent->dataSentReceived(0, bytes);
    } else if (bytes < 0) {
      // We hit an error. close.
      m_inSocket->close();
    }
  });

  connect(m_outSocket, &QTcpSocket::disconnected, this,
          [this]() { m_inSocket->close(); });

  connect(m_outSocket, &QTcpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError error) {
            if (m_state != Proxy) {
              ServerResponsePacket packet(createServerResponsePacket(
                  socketErrorToSocks5Rep(error), 0x00));
              m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
            }
            m_inSocket->close();
          });
}

uint8_t Socks5Client::socketErrorToSocks5Rep(
    QAbstractSocket::SocketError error) {
  switch (error) {
    case QAbstractSocket::HostNotFoundError:
      return 0x04;  // Host unreachable

    case QAbstractSocket::ConnectionRefusedError:
      return 0x05;  // Connection refused

    default:
      return 0x01;  // general SOCKS server failure
  }
}
