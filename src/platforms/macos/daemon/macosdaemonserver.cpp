/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemonserver.h"

#include <QCoreApplication>

#include "commandlineparser.h"
#include "leakdetector.h"
#include "macosdaemon.h"
#include "signalhandler.h"
#include "xpcdaemonserver.h"

MacOSDaemonServer::MacOSDaemonServer(QObject* parent)
    : Command(parent, "macosdaemon", "Activate the macos daemon") {
  MZ_COUNT_CTOR(MacOSDaemonServer);
}

MacOSDaemonServer::~MacOSDaemonServer() { MZ_COUNT_DTOR(MacOSDaemonServer); }

int MacOSDaemonServer::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  qputenv("QT_EVENT_DISPATCHER_CORE_FOUNDATION", "1");

  QString appName = tokens[0];
  QCoreApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  if (tokens.length() > 1) {
    QList<CommandLineParser::Option*> options;
    return CommandLineParser::unknownOption(this, appName, tokens[1], options,
                                            false);
  }

  // Create the daemon and its XPC service handler.
  MacOSDaemon daemon;
  new XpcDaemonServer(&daemon);

  // Signal handling for a proper shutdown.
  SignalHandler sh;
  QObject::connect(&sh, &SignalHandler::quitRequested,
                   [&]() { daemon.deactivate(); });
  QObject::connect(&sh, &SignalHandler::quitRequested, &app,
                   &QCoreApplication::quit, Qt::QueuedConnection);

  return app.exec();
}

static Command::RegistrationProxy<MacOSDaemonServer> s_commandMacOSDaemon;
