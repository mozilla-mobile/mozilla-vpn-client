/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemonlocalserver.h"
#include "daemonlocalserverconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>

#ifdef MVPN_MACOS
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>

constexpr const char* TMP_PATH = "/tmp/mozillavpn.socket";
constexpr const char* VAR_PATH = "/var/run/mozillavpn/daemon.socket";
#endif

namespace {
Logger logger(LOG_MAIN, "DaemonLocalServer");
}  // namespace

DaemonLocalServer::DaemonLocalServer(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(DaemonLocalServer);
}

DaemonLocalServer::~DaemonLocalServer() { MVPN_COUNT_DTOR(DaemonLocalServer); }

bool DaemonLocalServer::initialize() {
  m_server.setSocketOptions(QLocalServer::WorldAccessOption);

  QString path = daemonPath();
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

QString DaemonLocalServer::daemonPath() const {
#if defined(MVPN_WINDOWS)
  return "\\\\.\\pipe\\mozillavpn";
#elif defined(MVPN_MACOS)
  QDir dir("/var/run");
  if (!dir.exists()) {
    logger.log() << "/var/run doesn't exist. Fallback /tmp.";
    return TMP_PATH;
  }

  if (dir.exists("mozillavpn")) {
    logger.log() << "/var/run/mozillavpn seems to be usable";
    return VAR_PATH;
  }

  if (!dir.mkdir("mozillavpn")) {
    logger.log() << "Failed to create /var/run/mozillavpn";
    return TMP_PATH;
  }

  if (chmod("/var/run/mozillavpn", S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
    logger.log()
        << "Failed to set the right permissions to /var/run/mozillavpn";
    return TMP_PATH;
  }

  return VAR_PATH;
#else
#  error Unsupported platform
#endif
}
