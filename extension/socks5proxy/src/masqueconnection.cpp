/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "masqueconnection.h"

#include <QNetworkDatagram>
#include <QUdpSocket>
#include <QUrl>
#include <QtEndian>

#include "dnsresolver.h"

constexpr const uint MASQUE_CAPSULE_DATAGRAM = 0;

// Peek at the socket and determine if this is a MASQUE connection.
bool MasqueConnection::isProxyType(QIODevice* socket) {
  if (!socket->canReadLine()) {
    return false;
  }

  // Read one line and then roll it back.
  socket->startTransaction();
  auto guard = qScopeGuard([socket]() { socket->rollbackTransaction(); });
  QString line = QString(socket->readLine());

  auto match = m_requestRegex.match(line);
  if (!match.hasMatch() || match.lastCapturedIndex() != 3) {
    return false;
  }

  return (match.captured(1) == "GET") &&
         match.captured(2).contains(".well-known/masque/udp/");
}

void MasqueConnection::onHttpGet() {
  // Resolve the URL and parse the destination from the path.
  QUrl fullUrl = m_baseUrl.resolved(QUrl(m_uri, QUrl::StrictMode));
  QStringList path = fullUrl.path().split('/', Qt::SkipEmptyParts);

  // Validate that it matches our template.
  if ((path.length() != 5) || (path.at(0) != ".well-known") ||
      (path.at(1) != "masque") || (path.at(2) != "udp")) {
    setError(404, "Not Found");
    return;
  }

  // RFC9298 Section 3.2:
  // The request shall include a Connection header with value "upgrade"
  // The request shall include an Upgrade header with value "connect-udp"
  if (header("Connection").compare("Upgrade", Qt::CaseInsensitive) != 0) {
    setError(400, "Bad Request");
    return;
  }
  if (header("Upgrade").compare("connect-udp", Qt::CaseInsensitive) != 0) {
    setError(400, "Bad Request");
    return;
  }

  // Parse the destination and port.
  m_destPort = path.at(4).toUInt();
  QHostAddress address;
  if (address.setAddress(path.at(3))) {
    onHostnameResolved(address);
    return;
  }

  // Perform DNS resolution.
  m_destHostname = path.at(3);
  setState(Resolve);
  DNSResolver::instance()->resolveAsync(m_destHostname, this);
}

void MasqueConnection::onHostnameResolved(const QHostAddress& resolved) {
  if (m_destSocket != nullptr) {
    // We might get multiple ip results.
    return;
  }
  Q_ASSERT(!resolved.isNull());

  m_destAddress = resolved;
  m_destSocket = createDestSocket<QUdpSocket>(m_destAddress, m_destPort);

  connect(m_destSocket, &QUdpSocket::connected, this, [this]() {
    QMap<QString, QString> values;
    values["Connection"] = "Upgrade";
    values["Upgrade"] = "connect-udp";
    sendResponse(101, "Switching Protocols", values);
    setState(Proxy);
  });

  connect(m_destSocket, &QUdpSocket::errorOccurred, this,
          [this](QAbstractSocket::SocketError error) {
            qDebug() << "dest socket error:" << m_destSocket->errorString();
          });

  m_destSocket->connectToHost(m_destAddress, m_destPort);
}

bool MasqueConnection::readVarInt(quint64& value) {
  quint8 first;
  if (!m_clientSocket->peek((char*)&first, 1)) {
    return false;
  }
  quint8 len = 1 << (first >> 6);
  QByteArray buf = m_clientSocket->read(len);
  if (buf.length() != len) {
    return false;
  }
  value = 0;
  buf[0] &= 0x3F;
  for (quint8 x : buf) {
    value = (value << 8) + x;
  }
  return true;
}

void MasqueConnection::putVarInt(quint64 value, QByteArray& buffer) {
  if ((value >> 6) == 0) {
    quint8 v = value;
    buffer.append(v);
  } else if ((value >> 14) == 0) {
    quint16 v = qToBigEndian<quint16>(value | (0x1u << 14));
    buffer.append((char*)&v, sizeof(v));
  } else if ((value >> 30) == 0) {
    quint32 v = qToBigEndian<quint32>(value | (0x2u << 30));
    buffer.append((char*)&v, sizeof(v));
  } else {
    quint64 v = qToBigEndian(value | 0x3ull << 62);
    buffer.append((char*)&v, sizeof(v));
  }
}

void MasqueConnection::clientProxyRead() {
  while (true) {
    if (m_capsuleLength == 0) {
      // Read the next capsule header.
      m_clientSocket->startTransaction();
      quint64 type;
      quint64 len;
      if (!readVarInt(type) || !readVarInt(len)) {
        m_clientSocket->rollbackTransaction();
        return;
      }
      m_clientSocket->commitTransaction();

      // Reset the buffer.
      m_capsuleType = type;
      m_capsuleLength = len;
      m_capsuleBuffer.clear();

      // If the length exceeds the max buffer size, drop it.
      m_capsuleDrop = (len > MAX_CONNECTION_BUFFER);
      if (!m_capsuleDrop) {
        m_capsuleBuffer.reserve(len);
      }
    } else if (m_capsuleDrop) {
      // Discard unsupportable capsules.
      qint64 sz = m_clientSocket->skip(m_capsuleLength);
      if (sz <= 0) {
        return;
      }
      m_capsuleLength -= sz;
    } else if (m_capsuleBuffer.length() < m_capsuleLength) {
      // Read more data into the capsule buffer.
      quint64 needed = m_capsuleLength - m_capsuleBuffer.length();
      QByteArray chunk = m_clientSocket->read(needed);
      if (chunk.isEmpty()) {
        return;
      }
      m_capsuleBuffer.append(chunk);
    } else {
      // Handle the capsule.
      handleCapsule(m_capsuleType, m_capsuleBuffer);
      m_capsuleType = 0;
      m_capsuleLength = 0;
      m_capsuleBuffer.clear();
    }
  }
}

void MasqueConnection::destProxyRead() {
  QUdpSocket* socket = qobject_cast<QUdpSocket*>(m_destSocket);
  Q_ASSERT(socket);

  while (socket->hasPendingDatagrams()) {
    auto dgram = socket->receiveDatagram();
    if (dgram.isNull()) {
      return;
    }

    QByteArray data = dgram.data();
    QByteArray capsule;
    putVarInt(MASQUE_CAPSULE_DATAGRAM, capsule);
    putVarInt(data.length(), capsule);

    qsizetype available =
        MAX_CONNECTION_BUFFER - m_clientSocket->bytesToWrite();
    if ((data.length() + capsule.length()) > available) {
      // Drop the datagram if it would overflow the connection buffer.
      continue;
    }

    // Send the datagram capsule.
    m_clientSocket->write(capsule);
    m_clientSocket->write(data);
  }
}

void MasqueConnection::handleCapsule(quint64 type, const QByteArray& data) {
  switch (type) {
    case MASQUE_CAPSULE_DATAGRAM:
      // DATAGRAM capsule
      m_destSocket->write(data);
      break;

    default:
      // Unsupported capsule.
      break;
  }
}
