/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
#include <QStringList>

#include "webextbridge.h"
#include "webexthandler.h"

constexpr const int BRIDGE_RETRY_DELAY = 500;
constexpr const int BRIDGE_START_DELAY = 10;

WebExtBridge::WebExtBridge(const QString& name, QObject* parent)
    : QObject(parent), m_name(name) {

  connect(&m_socket, &QLocalSocket::stateChanged, this,
          &WebExtBridge::stateChanged);

  connect(&m_socket, &QLocalSocket::errorOccurred, this,
          &WebExtBridge::errorOccurred);

  connect(&m_socket, &QIODevice::bytesWritten, this,
          &WebExtBridge::tryPushData);

  connect(&m_retryTimer, &QTimer::timeout, this,
          &WebExtBridge::retryConnection);

  // Upon disconnection - attempt to retry after a short delay.
  m_retryTimer.setSingleShot(true);
  m_retryTimer.start(BRIDGE_START_DELAY);
}

void WebExtBridge::stateChanged(QLocalSocket::LocalSocketState state) {
  if (state == QLocalSocket::ConnectedState) {
    Q_ASSERT(m_reader == nullptr);
    m_reader = new WebExtReader(&m_socket, this);
    connect(&m_socket, &QIODevice::readyRead, m_reader,
            &WebExtReader::readyRead);
    connect(m_reader, &WebExtReader::messageReceived, this,
            [&](const QByteArray& data) { emit messageReceived(data); });

    emit connected();
  } else if (m_reader != nullptr) {
    delete m_reader;
    m_reader = nullptr;
    emit disconnected();
  }

  // Upon disconnection - attempt to retry after a short delay.
  if (state == QLocalSocket::UnconnectedState) {
    m_retryTimer.start(BRIDGE_RETRY_DELAY);
  }
}

void WebExtBridge::errorOccurred(QLocalSocket::LocalSocketError socketError) {
  //qInfo() << "Web extension socket error:" << m_socket.errorString();
}

void WebExtBridge::retryConnection() {
  // Abort and try to reconnect.
  if (m_socket.state() != QLocalSocket::UnconnectedState) {
    m_socket.abort();
  }
  m_socket.connectToServer(m_name);
}

bool WebExtBridge::sendMessage(const QByteArray& message) {
  if (m_socket.state() != QLocalSocket::ConnectedState) {
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
