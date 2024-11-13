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

constexpr const int MAX_CONNECTION_BUFFER = 16 * 1024;

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
  connect(socket, &QTcpSocket::disconnected, this,
          [this]() { setState(Closed); });

  connect(socket, &QTcpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError error) {
            if (error == QAbstractSocket::RemoteHostClosedError) {
              setState(Closed);
            } else {
              setError(ErrorGeneral, m_inSocket->errorString());
            }
          });

  socket->setReadBufferSize(MAX_CONNECTION_BUFFER);

  m_socksPort = socket->localPort();
  m_clientName = socket->peerAddress().toString();
}

Socks5Connection::Socks5Connection(QLocalSocket* socket)
    : Socks5Connection(static_cast<QIODevice*>(socket)) {
  connect(socket, &QLocalSocket::disconnected, this,
          [this]() { setState(Closed); });

  connect(socket, &QLocalSocket::errorOccurred, this,
          [this](QLocalSocket::LocalSocketError error) {
            if (error == QLocalSocket::PeerClosedError) {
              setState(Closed);
            } else {
              setError(ErrorGeneral, m_inSocket->errorString());
            }
          });

  socket->setReadBufferSize(MAX_CONNECTION_BUFFER);

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
      m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket));
      break;
    }

    default:
      break;
  }

  m_errorString = errorString;
  setState(Closed);
}

void Socks5Connection::setState(Socks5State newstate) {
  m_state = newstate;
  emit stateChanged();

  // If the state is closing. Shutdown the sockets.
  if (m_state == Closed) {
    m_inSocket->close();
    if (m_outSocket != nullptr) {
      m_outSocket->close();
    }

    // Request self-destruction
    deleteLater();
  }
}

void Socks5Connection::readyRead() {
  switch (m_state) {
    case ClientGreeting: {
      const auto packet = readPacket<ClientGreetingPacket>(m_inSocket);
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
      if (m_inSocket->bytesAvailable() < (qint64)m_authNumber) {
        return;
      }

      char buffer[INT8_MAX];
      if (m_inSocket->read(buffer, m_authNumber) != m_authNumber) {
        setError(ErrorGeneral, m_inSocket->errorString());
        return;
      }

      ServerChoicePacket packet;
      packet.version = 0x5;
      packet.cauth = 0x00;  // TODO: authentication check!

      if (m_inSocket->write((const char*)&packet, sizeof(ServerChoicePacket)) !=
          sizeof(ServerChoicePacket)) {
        setError(ErrorGeneral, m_inSocket->errorString());
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
          setError(ErrorGeneral, m_inSocket->errorString());
          return;
        }

        char buffer[UINT8_MAX];
        if (m_inSocket->read(buffer, length) != length) {
          setError(ErrorGeneral, m_inSocket->errorString());
          return;
        }

        uint16_t port;
        if (m_inSocket->read((char*)&port, sizeof(port)) != sizeof(port)) {
          setError(ErrorGeneral, m_inSocket->errorString());
          return;
        }

        // Todo VPN-6510: Let's resolve the Host with the local device DNS
        QString hostname = QString::fromUtf8(buffer, length);
        m_hostLookupStack.append(hostname);

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
        setError(ErrorAddressNotSupported, "Address type not supported");
        return;
      }
    }

    break;

    case Proxy:
      proxy(m_inSocket, m_outSocket, m_sendHighWaterMark);
      break;

    default:
      Q_ASSERT(false);
      break;
  }
}

void Socks5Connection::proxy(QIODevice* from, QIODevice* to, quint64 &watermark) {
  Q_ASSERT(from && to);

  for (;;) {
    qint64 available = from->bytesAvailable();
    if (available <= 0) {
      break;
    }

    qint64 capacity = MAX_CONNECTION_BUFFER - to->bytesToWrite();
    if (capacity <= 0) {
      break;
    }

    QByteArray data = from->read(qMin(available, capacity));
    if (data.length() == 0) {
      break;
    }
    qint64 sent = to->write(data);
    if (sent != data.length()) {
      qDebug() << "Truncated write. Sent" << sent << "of" << data.length();
      break;
    }
  }

  // Update buffer high watermark.
  qint64 queued = to->bytesToWrite();
  if (queued > watermark) {
    watermark = queued;
  }
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
    setError(ErrorHostUnreachable, lookup->errorString());
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
    setError(ErrorHostUnreachable, "Maximum DNS attempts exhausted");
    return;
  }

  // If we got a canonical name record, restart the lookup using the CNAME.
  auto cnameRecords = lookup->canonicalNameRecords();
  if (cnameRecords.length() > 0) {
    QString target = cnameRecords.first().value();
    m_hostLookupStack.append(target);
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
    setError(ErrorHostUnreachable, "SRV records not supported");
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
  setError(ErrorHostUnreachable, "DNS hostname not found");
}

void Socks5Connection::configureOutSocket(quint16 port) {
  Q_ASSERT(!m_destAddress.isNull());
  m_hostLookupStack.append(m_destAddress.toString());
  m_outSocket = new QTcpSocket(this);
  emit setupOutSocket(m_outSocket, m_destAddress);

  m_outSocket->connectToHost(m_destAddress, port);

  connect(m_outSocket, &QTcpSocket::connected, this, [this]() {
    setState(Proxy);
    ServerResponsePacket packet(createServerResponsePacket(0x00, m_socksPort));
    if (m_inSocket->write((char*)&packet, sizeof(ServerResponsePacket)) !=
        sizeof(ServerResponsePacket)) {
      setError(ErrorGeneral, m_inSocket->errorString());
      return;
    }

    readyRead();
  });

  connect(m_outSocket, &QIODevice::bytesWritten, this, [this](qint64 bytes) {
    emit dataSentReceived(0, bytes);
    proxy(m_inSocket, m_outSocket, m_sendHighWaterMark);
  });
  connect(m_inSocket, &QIODevice::bytesWritten, this, [this](qint64 bytes) {
    emit dataSentReceived(bytes, 0);
    proxy(m_outSocket, m_inSocket, m_recvHighWaterMark);
  });

  connect(m_outSocket, &QTcpSocket::readyRead, this, [this]() {
    proxy(m_outSocket, m_inSocket, m_recvHighWaterMark);
  });

  connect(m_outSocket, &QTcpSocket::disconnected, this,
          [this]() { setState(Closed); });

  connect(m_outSocket, &QTcpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError error) {
            if (error == QAbstractSocket::RemoteHostClosedError) {
              setState(Closed);
            } else {
              setError(ErrorGeneral, m_outSocket->errorString());
            }
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
