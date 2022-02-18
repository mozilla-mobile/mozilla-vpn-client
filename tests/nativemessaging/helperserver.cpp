/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helperserver.h"

#include <QTcpSocket>

void HelperServer::start() {
  Q_ASSERT(!m_server);
  m_thread.start();

  m_server = new EchoServer();
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

void EchoServer::start() {
  if (!listen(QHostAddress::Any, 8754)) {
    qFatal("Failed to listen to port 8754");
    return;
  }

  connect(this, &QTcpServer::newConnection, [this]() {
    QTcpSocket* socket = nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, [socket]() {
      socket->write(socket->readAll());
      socket->flush();
    });
  });

  emit ready();
}
