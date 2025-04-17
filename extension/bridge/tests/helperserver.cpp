/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helperserver.h"

void HelperServer::start(int fuzzy) {
  Q_ASSERT(!m_server);
  m_thread.start();

  m_server = new EchoServer(fuzzy);
  QObject::connect(this, &HelperServer::startServer, m_server,
                   &EchoServer::start);
  QObject::connect(m_server, &EchoServer::ready, this, &HelperServer::ready);

  m_server->moveToThread(&m_thread);

  emit startServer();
}

void HelperServer::stop() {
  Q_ASSERT(m_server);
  m_server->deleteLater();

  m_thread.quit();
  m_thread.wait();
}

EchoServer::EchoServer(int fuzzy) : m_fuzzy(fuzzy) {}

void EchoServer::start() {
  if (!listen(QHostAddress::Any, 8754)) {
    qFatal("Failed to listen to port 8754");
    return;
  }

  connect(this, &QTcpServer::newConnection,
          [this]() { new EchoConnection(nextPendingConnection(), m_fuzzy); });

  emit ready();
}

EchoConnection::EchoConnection(QTcpSocket* socket, int fuzzy)
    : m_socket(socket), m_fuzzy(fuzzy) {
  m_timer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, &m_timer, [this]() {
    Q_ASSERT(!m_buffer.isEmpty());
    m_socket->write(m_buffer.constData(), 1);
    m_socket->flush();
    m_buffer.remove(0, 1);
    maybeStartTimer();
  });

  connect(m_socket, &QTcpSocket::readyRead, m_socket, [this]() {
    QByteArray buffer = m_socket->readAll();
    if (!m_fuzzy) {
      m_socket->write(buffer);
      m_socket->flush();
      return;
    }

    m_buffer.append(buffer);
    maybeStartTimer();
  });
}

void EchoConnection::maybeStartTimer() {
  if (!m_buffer.isEmpty() && !m_timer.isActive()) {
    m_timer.start(m_fuzzy);
  }
}
