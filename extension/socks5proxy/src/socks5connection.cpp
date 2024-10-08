/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "socks5connection.h"

#include "socks5.h"

#ifdef Q_OS_WIN
#  include <winsock2.h>
#  include <ws2ipdef.h>
#else
#  include <arpa/inet.h>
#endif

#include <QDnsLookup>
#include <QHostAddress>

constexpr const int MAX_DNS_LOOKUP_ATTEMPTS = 5;

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

Socks5Connection::Socks5Connection(QIODevice* socket)
    : QObject(socket), m_inSocket(socket) {
  connect(m_inSocket, &QIODevice::readyRead, this,
          &Socks5Connection::readyRead);
  readyRead();
}

Socks5Connection::Socks5Connection(QTcpSocket* socket)
    : Socks5Connection(static_cast<QIODevice*>(socket)) {
  connect(socket, &QTcpSocket::disconnected, this, &QObject::deleteLater);

  m_socksPort = socket->localPort();
  m_clientName = socket->peerAddress().toString();
}

Socks5Connection::Socks5Connection(QLocalSocket* socket)
    : Socks5Connection(static_cast<QIODevice*>(socket)) {
  connect(socket, &QLocalSocket::disconnected, this, &QObject::deleteLater);

  // TODO: Some magic may be required here to resolve the entity of which client
  // tried to connect. Some breadcrumbs:
  //   - Linux: SO_PEERCRED can get us the cllient PID, from which we can get
  //            the cgroup name, systemd scope and parse out the application ID.
  //   - Windows: GetNamedPipeClientProcessId() and GetProcessImageFileNameA()
  //              can get us the path to the calling executable.
  //   - MacOS: SecTaskCopySigningIdentifier() can be used to grab information
  //            about processes and their code signatures. Somewhere in there
  //            I would expect to find the application ID too.
  m_clientName = localClientName(socket);
}

void Socks5Connection::setState(Socks5State newstate) {
  m_state = newstate;
  emit stateChanged();
}

void Socks5Connection::readyRead() {
  switch (m_state) {
    case ClientGreeting: {
      const auto packet = readPacket<ClientGreetingPacket>(m_inSocket);
      if (!packet) {
        return;
      }
      if (packet.value().version != 0x5) {
        // We only currently want to support socks5.
        // as otherwise we could not support udp or auth.
        ServerResponsePacket packet(createServerResponsePacket(ErrorGeneral));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }
      m_authNumber = packet.value().nauth;
      setState(AuthenticationMethods);
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

      setState(ClientConnectionRequest);
      [[fallthrough]];
    }

    case ClientConnectionRequest: {
      auto const packet = readPacket<ClientConnectionRequestPacket>(m_inSocket);
      if (!packet) {
        return;
      }
      if (packet.value().version != 0x5 || packet.value().rsv != 0x00) {
        ServerResponsePacket packet(createServerResponsePacket(ErrorGeneral));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }
      if (packet.value().cmd != 0x01u /* connection */) {
        ServerResponsePacket packet(
            createServerResponsePacket(ErrorCommandNotSupported));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }
      m_addressType = packet.value().atype;
      setState(ClientConnectionAddress);
      [[fallthrough]];
    }

    case ClientConnectionAddress: {
      if (m_addressType == 0x01 /* Ipv4 */) {
        auto const packet =
            readPacket<ClientConnectionAddressIpv4Packet>(m_inSocket);
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
        if (m_inSocket->bytesAvailable() == 0) return;

        uint8_t length;
        if (m_inSocket->read((char*)&length, 1) != 1) {
          m_inSocket->close();
          return;
        }

        char buffer[UINT8_MAX];
        if (m_inSocket->read(buffer, length) != length) {
          m_inSocket->close();
          return;
        }

        uint16_t port;
        if (m_inSocket->read((char*)&port, sizeof(port)) != sizeof(port)) {
          m_inSocket->close();
          return;
        }

        // Todo VPN-6510: Let's resolve the Host with the local device DNS
        QString hostname = QString::fromUtf8(buffer, length);
        m_dnsLookupStack.append(hostname);

        // Start a DNS lookup to handle this request.
        m_dnsLookupAttempts = MAX_DNS_LOOKUP_ATTEMPTS;
        QDnsLookup* lookup = new QDnsLookup(QDnsLookup::ANY, hostname, this);
        connect(lookup, &QDnsLookup::finished, this,
                [this, port]() { dnsResolutionFinished(htons(port)); });

        lookup->lookup();
      }

      else if (m_addressType == 0x04 /* Ipv6 */) {
        auto const packet =
            readPacket<ClientConnectionAddressIpv6Packet>(m_inSocket);
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
        ServerResponsePacket packet(
            createServerResponsePacket(ErrorAddressNotSupported));
        m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
        m_inSocket->close();
        return;
      }
    }

    break;

    case Proxy: {
      qint64 bytes = proxy(m_inSocket, m_outSocket);
      if (bytes) {
        emit dataSentReceived(bytes, 0);
      }
    } break;

    default:
      Q_ASSERT(false);
      break;
  }
}

// TODO: VPN-6513 make sure we cannot drop bytes even under pressure.
qint64 Socks5Connection::proxy(QIODevice* a, QIODevice* b) {
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

void Socks5Connection::dnsResolutionFinished(quint16 port) {
  QDnsLookup* lookup = qobject_cast<QDnsLookup*>(QObject::sender());

  // Garbage collect the lookup when we're finished.
  m_dnsLookupAttempts--;
  auto guard = qScopeGuard([lookup]() {
    if (lookup->isFinished()) {
      lookup->deleteLater();
    }
  });

  if (lookup->error() != QDnsLookup::NoError) {
    qDebug() << "Received error:" << lookup->errorString();
    ServerResponsePacket packet(
        createServerResponsePacket(ErrorHostUnreachable));
    m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
    m_inSocket->close();
    return;
  }

  // If we get a hostname record. Then DNS resolution has succeeded. and
  // we can proceed to the outbound socket setup.
  auto hostRecords = lookup->hostAddressRecords();
  if (hostRecords.length() > 0) {
    m_destAddress = hostRecords.first().value();
    configureOutSocket(port);
    return;
  }

  // If we have exhausted the DNS lookup attempts, then give up on DNS
  // resolution and report the host as unreachable. This safeguards us from
  // recursive DNS loops and other unresolvable situations.
  if (m_dnsLookupAttempts <= 0) {
    ServerResponsePacket packet(
        createServerResponsePacket(ErrorHostUnreachable));
    m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
    m_inSocket->close();
    return;
  }

  // If we got a canonical name record, restart the lookup using the CNAME.
  auto cnameRecords = lookup->canonicalNameRecords();
  if (cnameRecords.length() > 0) {
    QString target = cnameRecords.first().value();
    m_dnsLookupStack.append(target);
    lookup->setName(target);
    lookup->setType(QDnsLookup::ANY);
    lookup->lookup();
    return;
  }

  // Service records are not supported.
  auto serviceRecords = lookup->serviceRecords();
  if (serviceRecords.length() > 0) {
    // TODO: Not supported.
    //
    // In theory we can restart the DNS lookup with the target name, but
    // the port may have changed too and that is stored somewhere in the
    // signal binding. Service records aren't used a whole lot out in the
    // wild either.
    //
    // We can also receive more than one service record and we are expected
    // to load balance/fallback amongst them.
    ServerResponsePacket packet(
        createServerResponsePacket(ErrorHostUnreachable));
    m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
    m_inSocket->close();
    return;
  }

  // If we get this far, the request didn't fail, but we also didn't get any
  // records that we could make sense of. Fallback to an explicit IPv4 (A) query
  // if this originated from an ANY query.
  if (lookup->type() == QDnsLookup::ANY) {
    lookup->setType(QDnsLookup::A);
    lookup->lookup();
    return;
  }

  // Otherwise, no such host was found.
  ServerResponsePacket packet(createServerResponsePacket(ErrorHostUnreachable));
  m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
  m_inSocket->close();
}

void Socks5Connection::configureOutSocket(quint16 port) {
  Q_ASSERT(!m_destAddress.isNull());
  m_outSocket = new QTcpSocket(this);
  emit setupOutSocket(m_outSocket, m_destAddress);

  m_outSocket->connectToHost(m_destAddress, port);

  connect(m_outSocket, &QTcpSocket::connected, this, [this]() {
    ServerResponsePacket packet(createServerResponsePacket(0x00, m_socksPort));
    if (m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket)) !=
        sizeof(ServerResponsePacket)) {
      m_inSocket->close();
      return;
    }

    setState(Proxy);
    readyRead();
  });

  connect(m_outSocket, &QTcpSocket::readyRead, this, [this]() {
    qint64 bytes = proxy(m_outSocket, m_inSocket);
    if (bytes > 0) {
      emit dataSentReceived(0, bytes);
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
              ServerResponsePacket packet(
                  createServerResponsePacket(socketErrorToSocks5Rep(error)));
              m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
            }
            m_inSocket->close();
          });
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
std::optional<T> Socks5Connection::readPacket(QIODevice* connection) {
  // There are not enough bytes to read don't touch the connection.
  if (connection->bytesAvailable() < (qint64)sizeof(T)) {
    return {};
  }
  connection->startTransaction();
  T packet;
  if (connection->read((char*)&packet, sizeof(T)) != sizeof(T)) {
    // If we did not read the correct amount of data
    // Abort the transaction and reset the buffer, so the
    // caller can try to read again.
    connection->rollbackTransaction();
    return {};
  }
  connection->commitTransaction();
  return packet;
}
