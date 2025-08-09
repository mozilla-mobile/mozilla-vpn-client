/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "proxyconnection.h"

#include <QLocalSocket>
#include <QTcpSocket>
#include <QTimer>

#ifdef Q_OS_WIN
#  include <winsock2.h>
#  include <ws2ipdef.h>

#  include "winutils.h"
#else
#  include <arpa/inet.h>
#endif

ProxyConnection::ProxyConnection(QIODevice* socket)
    : QObject(socket), m_clientSocket(socket) {
  connect(m_clientSocket, &QIODevice::readyRead, this,
          &ProxyConnection::clientReadyRead);
  connect(m_clientSocket, &QIODevice::bytesWritten, this,
          &ProxyConnection::clientBytesWritten);

  // Handle TCP/UDP socket setup.
  QAbstractSocket* netsock = qobject_cast<QAbstractSocket*>(socket);
  if (netsock) {
    m_clientPort = netsock->localPort();
    m_clientName = netsock->peerAddress().toString();

    connect(netsock, &QAbstractSocket::disconnected, this,
            [this]() { setState(Closed); });
    connect(netsock, &QAbstractSocket::errorOccurred, this,
            &ProxyConnection::clientErrorOccurred<QAbstractSocket>);

    netsock->setReadBufferSize(MAX_CONNECTION_BUFFER);
  }

  // Handle UNIX/Local socket setup.
  QLocalSocket* local = qobject_cast<QLocalSocket*>(socket);
  if (local) {
    m_clientPort = 0;
    m_clientName = localClientName(local);

    connect(local, &QLocalSocket::disconnected, this,
            [this]() { setState(Closed); });
    connect(local, &QLocalSocket::errorOccurred, this,
            &ProxyConnection::clientErrorOccurred<QLocalSocket>);

    local->setReadBufferSize(MAX_CONNECTION_BUFFER);
  }

  // Immediately trigger the readyRead signal.
  QTimer::singleShot(0, this, &ProxyConnection::clientReadyRead);
}

void ProxyConnection::proxy(QIODevice* from, QIODevice* to,
                            quint64& watermark) {
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

void ProxyConnection::clientReadyRead() {
  if (m_state >= Handshake) {
    handshakeRead();
  }

  if (m_state == Proxy) {
    clientProxyRead();
  }
}

void ProxyConnection::setState(int newstate) {
  if (m_state == newstate) {
    return;
  }

  m_state = newstate;
  emit stateChanged();

  if (m_state == Proxy) {
    // Keep track of how many bytes are yet to be written to finish the
    // negotiation. We should suppress the statistics signals for such traffic.
    m_recvIgnoreBytes = m_clientSocket->bytesToWrite();

    // Kick the flow control once we are ready to start passing traffic.
    clientReadyRead();
  }

  // If the state is closing. Shutdown the sockets.
  if (m_state == Closed) {
    emit disconnected();

    m_clientSocket->close();
    if (m_destSocket != nullptr) {
      m_destSocket->close();
    }

    // Request self-destruction
    deleteLater();
  }
}

template <typename T>
void ProxyConnection::clientErrorOccurred(int error) {
  auto socket = qobject_cast<T*>(QObject::sender());
  if (socket && error != QAbstractSocket::RemoteHostClosedError) {
    m_errorString = socket->errorString();
  }

  setState(Closed);
}

void ProxyConnection::clientBytesWritten(qint64 bytes) {
  // Ignore this signal outside of the proxy state.
  if (m_state != Proxy) {
    return;
  }

  // Ignore this signal if it's just reporting a negotiation packet.
  if (m_recvIgnoreBytes >= bytes) {
    m_recvIgnoreBytes -= bytes;
    return;
  } else if (m_recvIgnoreBytes > 0) {
    bytes -= m_recvIgnoreBytes;
    m_recvIgnoreBytes = 0;
  }

  // Drive statistics and proxy data more data, if available.
  emit dataSentReceived(0, bytes);
  destProxyRead();
}

// The default implementation - just calls proxy().
void ProxyConnection::clientProxyRead() {
  proxy(m_clientSocket, m_destSocket, m_sendHighWaterMark);
}

// The default implementation - just calls proxy().
void ProxyConnection::destProxyRead() {
  proxy(m_destSocket, m_clientSocket, m_recvHighWaterMark);
}

QAbstractSocket* ProxyConnection::createDestSocketImpl(QAbstractSocket* sock,
                                                       const QHostAddress& dest,
                                                       quint16 port) {
  Q_ASSERT(!dest.isNull());
  Q_ASSERT(sock != nullptr);
  auto guard = qScopeGuard([sock]() { sock->deleteLater(); });

  int family;
  if (dest.protocol() == QAbstractSocket::IPv6Protocol) {
    family = AF_INET6;
  } else {
    family = AF_INET;
  }

  // The platform layer might want to fiddle with the socket before we connect,
  // but the socket descriptor is typically created inside the connectToHost()
  // method. So let's create the socket manually and emit a signal for the
  // platform logic to hook on.
  qintptr newsock;
  if (sock->socketType() == QAbstractSocket::UdpSocket) {
    newsock = socket(family, SOCK_DGRAM, 0);
  } else {
    newsock = socket(family, SOCK_STREAM, IPPROTO_TCP);
  }
#ifdef Q_OS_WIN
  if (newsock == INVALID_SOCKET) {
    m_errorString = WinUtils::win32strerror(WSAGetLastError());
    return nullptr;
  }
#else
  if (newsock < 0) {
    m_errorString = strerror(errno);
    return nullptr;
  }
#endif
  emit setupOutSocket(newsock, m_destAddress);

  sock->setSocketDescriptor(newsock, QAbstractSocket::UnconnectedState);
  sock->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

  connect(sock, &QIODevice::readyRead, this, [this]() { destProxyRead(); });
  connect(sock, &QIODevice::bytesWritten, this,
          &ProxyConnection::clientReadyRead, Qt::QueuedConnection);
  connect(sock, &QIODevice::bytesWritten, this,
          [this](qint64 bytes) { emit dataSentReceived(bytes, 0); });

  guard.dismiss();
  return sock;
}
