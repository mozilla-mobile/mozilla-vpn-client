/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "proxyconnection.h"

#include <QLocalSocket>
#include <QTcpSocket>

ProxyConnection::ProxyConnection(QIODevice* socket)
    : QObject(socket), m_clientSocket(socket) {
  connect(m_clientSocket, &QIODevice::readyRead, this,
          &ProxyConnection::readyRead);

  // Handle TCP/UDP socket setup.
  QAbstractSocket* netsock = qobject_cast<QAbstractSocket*>(socket);
  if (netsock) {
    m_clientPort = netsock->localPort();
    m_clientName = netsock->peerAddress().toString();

    connect(netsock, &QAbstractSocket::errorOccurred, this,
            &ProxyConnection::clientErrorOccurred<QAbstractSocket>);

    netsock->setReadBufferSize(MAX_CONNECTION_BUFFER);
  }

  // Handle UNIX/Local socket setup.
  QLocalSocket* local = qobject_cast<QLocalSocket*>(socket);
  if (local) {
    m_clientPort = 0;
    m_clientName = localClientName(local);

    connect(local, &QLocalSocket::errorOccurred, this,
            &ProxyConnection::clientErrorOccurred<QLocalSocket>);

    local->setReadBufferSize(MAX_CONNECTION_BUFFER);
  }
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

void ProxyConnection::readyRead() {
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

  // If the state is closing. Shutdown the sockets.
  if (m_state == Closed) {
    emit disconnected();
  
    m_clientSocket->close();
    //if (m_outSocket != nullptr) {
    //  m_outSocket->close();
    //}

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
