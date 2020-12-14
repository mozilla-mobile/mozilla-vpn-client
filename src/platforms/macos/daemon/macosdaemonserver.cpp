/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemonserver.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "logger.h"
#include "macosdaemon.h"
#include "macosdaemonconnection.h"
#include "macosutils.h"
#include "mozillavpn.h"
#include "signalhandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

constexpr const char* TMP_PATH = "/tmp/mozillavpn.socket";
constexpr const char* VAR_PATH = "/var/run/mozillavpn/daemon.socket";

namespace {
Logger logger(LOG_MACOS, "MacOSDaemonServer");
}

MacOSDaemonServer::MacOSDaemonServer(QObject* parent)
    : Command(parent, "macosdaemon", "Activate the macos daemon") {
  MVPN_COUNT_CTOR(MacOSDaemonServer);
}

MacOSDaemonServer::~MacOSDaemonServer() { MVPN_COUNT_DTOR(MacOSDaemonServer); }

int MacOSDaemonServer::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN Daemon");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  if (tokens.length() > 1) {
    QList<CommandLineParser::Option*> options;
    return CommandLineParser::unknownOption(this, appName, tokens[1], options,
                                            false);
  }

  m_server = new QLocalServer(this);
  m_server->setSocketOptions(QLocalServer::WorldAccessOption);

  QString path = daemonPath();
  logger.log() << "Server path:" << path;

  if (QFileInfo::exists(path)) {
    QFile::remove(path);
  }

  if (!m_server->listen(path)) {
    logger.log() << "Failed to listen the daemon path";
    return 1;
  }

  connect(m_server, &QLocalServer::newConnection, this,
          &MacOSDaemonServer::newConnection);

  MacOSDaemon daemon;

  // Signal handling for a proper shutdown.
  SignalHandler sh;
  QObject::connect(&sh, &SignalHandler::quitRequested,
                   []() { MacOSDaemon::instance()->deactivate(); });
  QObject::connect(&sh, &SignalHandler::quitRequested, &app,
                   &QCoreApplication::quit, Qt::QueuedConnection);

  return app.exec();
}

void MacOSDaemonServer::newConnection() {
  logger.log() << "New connection";

  if (!m_server->hasPendingConnections()) {
    return;
  }

  QLocalSocket* socket = m_server->nextPendingConnection();
  Q_ASSERT(socket);

  MacOSDaemonConnection* connection = new MacOSDaemonConnection(this, socket);
  connect(socket, &QLocalSocket::disconnected, connection,
          &MacOSDaemonConnection::deleteLater);
}

QString MacOSDaemonServer::daemonPath() const {
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
}

static Command::RegistrationProxy<MacOSDaemonServer> s_commandMacOSDaemon;
