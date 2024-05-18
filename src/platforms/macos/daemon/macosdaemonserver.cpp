/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemonserver.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocalServer>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "commandlineparser.h"
#include "constants.h"
#include "daemon/daemonlocalserverconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "macosdaemon.h"
#include "mozillavpn.h"
#include "signalhandler.h"

namespace {
Logger logger("MacOSDaemonServer");
}

MacOSDaemonServer::MacOSDaemonServer(QObject* parent)
    : Command(parent, "macosdaemon", "Activate the macos daemon") {
  MZ_COUNT_CTOR(MacOSDaemonServer);
}

MacOSDaemonServer::~MacOSDaemonServer() { MZ_COUNT_DTOR(MacOSDaemonServer); }

int MacOSDaemonServer::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  QCoreApplication::setApplicationName("Mozilla VPN Daemon");
  QCoreApplication::setApplicationVersion(Constants::versionString());

  if (tokens.length() > 1) {
    QList<CommandLineParser::Option*> options;
    return CommandLineParser::unknownOption(this, appName, tokens[1], options,
                                            false);
  }

  MacOSDaemon daemon;

  QLocalServer server(qApp);
  server.setSocketOptions(QLocalServer::WorldAccessOption);
  connect(&server, &QLocalServer::newConnection, [&]() {
    logger.debug() << "New connection received";
    if (!server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = server.nextPendingConnection();
    Q_ASSERT(socket);
    new DaemonLocalServerConnection(&daemon, socket);
  });

  QString path = daemonPath();
  if (QFileInfo::exists(path)) {
    QFile::remove(path);
  }
  if (!server.listen(path)) {
    logger.error() << "Failed to initialize the server";
    return 1;
  }

  // Signal handling for a proper shutdown.
  SignalHandler sh;
  QObject::connect(&sh, &SignalHandler::quitRequested,
                   []() { MacOSDaemon::instance()->deactivate(); });
  QObject::connect(&sh, &SignalHandler::quitRequested, &app,
                   &QCoreApplication::quit, Qt::QueuedConnection);

  return app.exec();
}

QString MacOSDaemonServer::daemonPath() {
  QDir dir("/var/run");
  if (!dir.exists()) {
    logger.warning() << "/var/run doesn't exist. Fallback /tmp.";
    return Constants::MACOS_DAEMON_TMP_PATH;
  }

  if (dir.exists("mozillavpn")) {
    logger.debug() << "/var/run/mozillavpn seems to be usable";
    return Constants::MACOS_DAEMON_VAR_PATH;
  }

  if (!dir.mkdir("mozillavpn")) {
    logger.warning() << "Failed to create /var/run/mozillavpn";
    return Constants::MACOS_DAEMON_TMP_PATH;
  }

  if (chmod("/var/run/mozillavpn", S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
    logger.warning()
        << "Failed to set the right permissions to /var/run/mozillavpn";
    return Constants::MACOS_DAEMON_TMP_PATH;
  }

  return Constants::MACOS_DAEMON_VAR_PATH;
}

static Command::RegistrationProxy<MacOSDaemonServer> s_commandMacOSDaemon;
