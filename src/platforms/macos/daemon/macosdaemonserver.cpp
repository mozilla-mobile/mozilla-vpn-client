/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemonserver.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDir>
#include <QLocalServer>

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

  QFileInfo path(Constants::MACOS_DAEMON_PATH);
  if (path.exists()) {
    QFile::remove(path.canonicalPath());
  } else if (!makeRuntimeDir(path.dir())) {
    logger.error() << "Failed to create runtime dir";
    return 1;
  }

  if (!server.listen(Constants::MACOS_DAEMON_PATH)) {
    logger.error() << "Failed to initialize the server";
    return 1;
  }

  // Signal handling for a proper shutdown.
  SignalHandler sh;
  QObject::connect(&sh, &SignalHandler::quitRequested,
                   [&]() { daemon.deactivate(); });
  QObject::connect(&sh, &SignalHandler::quitRequested, &app,
                   &QCoreApplication::quit, Qt::QueuedConnection);

  return app.exec();
}

bool MacOSDaemonServer::makeRuntimeDir(const QDir& dir) {
  if (dir.exists()) {
    return true;
  }

  int ret = mkdir(dir.absolutePath().toLocal8Bit().constData(),
                  S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  if (ret < 0) {
    logger.warning() << "Failed to create runtime dir:" << strerror(errno);
    return false;
  }

  return true;
}

static Command::RegistrationProxy<MacOSDaemonServer> s_commandMacOSDaemon;
