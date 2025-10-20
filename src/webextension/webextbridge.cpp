/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QHostAddress>
#include <QStringList>

#include "webextbridge.h"
#include "webexthandler.h"

constexpr const int BRIDGE_RETRY_DELAY = 500;

WebExtBridge::WebExtBridge(quint16 port, QObject* parent)
    : QObject(parent), m_port(port) {

  connect(&m_socket, &QAbstractSocket::stateChanged, this,
          &WebExtBridge::stateChanged);

  connect(&m_socket, &QAbstractSocket::errorOccurred, this,
          &WebExtBridge::errorOccurred);

  connect(&m_socket, &QIODevice::bytesWritten, this,
          &WebExtBridge::tryPushData);

  connect(&m_retryTimer, &QTimer::timeout, this,
          &WebExtBridge::retryConnection);

  // Upon disconnection - attempt to retry after a short delay.
  m_retryTimer.setSingleShot(true);
  connect(&m_socket, &QAbstractSocket::stateChanged, &m_retryTimer,
          [this](QAbstractSocket::SocketState state) {
            if (state == QAbstractSocket::UnconnectedState) {
              m_retryTimer.start(BRIDGE_RETRY_DELAY);
            }
          });

  // Immediately try to establish a connection.
  retryConnection();
}

void WebExtBridge::stateChanged(QAbstractSocket::SocketState state) {
  //qDebug() << "Web extension socket state:" << state;
  if (state == QAbstractSocket::ConnectedState) {
    Q_ASSERT(m_reader == nullptr);
    m_reader = new WebExtReader(&m_socket, this);
    connect(&m_socket, &QAbstractSocket::readyRead, m_reader,
            &WebExtReader::readyRead);
    connect(m_reader, &WebExtReader::messageReceived, this,
            [&](const QByteArray& data) { emit messageReceived(data); });

    emit connected();
  } else if (m_reader != nullptr) {
    delete m_reader;
    m_reader = nullptr;
    emit disconnected();
  }
}

void WebExtBridge::errorOccurred(QAbstractSocket::SocketError socketError) {
  //qDebug() << "Web extension socket error:" << m_socket.errorString();
}

void WebExtBridge::retryConnection() {
  // Abort and try to reconnect.
  if (m_socket.state() != QAbstractSocket::UnconnectedState) {
    m_socket.abort();
  }
  m_socket.connectToHost(QHostAddress(QHostAddress::LocalHost), m_port);
}

bool WebExtBridge::sendMessage(const QByteArray& message) {
  if (m_socket.state() != QAbstractSocket::ConnectedState) {
    return false;
  }

  quint32 length = message.length();
  m_buffer.append(reinterpret_cast<char*>(&length), sizeof(quint32));
  m_buffer.append(message);
  tryPushData();
  return true;
}

void WebExtBridge::tryPushData() {
  if (m_buffer.isEmpty()) {
    return;
  }
  qint64 len = m_socket.write(m_buffer);
  if (len > 0) {
    m_buffer = m_buffer.sliced(len);
  }
}
