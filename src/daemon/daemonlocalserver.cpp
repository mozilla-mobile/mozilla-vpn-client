/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemonlocalserver.h"
#include "daemonlocalserverconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QFileInfo>
#include <QLocalSocket>

namespace {
Logger logger(LOG_MAIN, "DaemonLocalServer");
}  // namespace

DaemonLocalServer::DaemonLocalServer(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(DaemonLocalServer);
}

DaemonLocalServer::~DaemonLocalServer() { MVPN_COUNT_DTOR(DaemonLocalServer); }

bool DaemonLocalServer::initialize() {
  m_server.setSocketOptions(QLocalServer::WorldAccessOption);

  QString path = "\\\\.\\pipe\\mozillavpn";
  logger.log() << "Server path:" << path;

  if (QFileInfo::exists(path)) {
    QFile::remove(path);
  }

  if (!m_server.listen(path)) {
    logger.log() << "Failed to listen the daemon path";
    return false;
  }

  connect(&m_server, &QLocalServer::newConnection, [&] {
    logger.log() << "New connection received";

    if (!m_server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);

    DaemonLocalServerConnection* connection =
        new DaemonLocalServerConnection(&m_server, socket);
    connect(socket, &QLocalSocket::disconnected, connection,
            &DaemonLocalServerConnection::deleteLater);
  });

  return true;
}
