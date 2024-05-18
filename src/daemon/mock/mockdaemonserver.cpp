/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mockdaemonserver.h"

#include <QCoreApplication>

#include "commandlineparser.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mockdaemon.h"
#include "mozillavpn.h"
#include "signalhandler.h"

namespace {
Logger logger("MockDaemonServer");
}

MockDaemonServer::MockDaemonServer(QObject* parent)
    : Command(parent, "mockdaemon", "Activate the mock daemon") {
  MZ_COUNT_CTOR(MockDaemonServer);
}

MockDaemonServer::~MockDaemonServer() { MZ_COUNT_DTOR(MockDaemonServer); }

int MockDaemonServer::run(QStringList& tokens) {
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

  QString path = qEnvironmentVariable("MVPN_CONTROL_SOCKET");
  if (path.isEmpty()) {
#ifdef MZ_WINDOWS
    path = Constants::WINDOWS_DAEMON_PATH;
#else
    path = Constants::MACOS_DAEMON_TMP_PATH;
#endif
  }

  MockDaemon daemon(path);

  // Signal handling for a proper shutdown.
  SignalHandler sh;
  QObject::connect(&sh, &SignalHandler::quitRequested,
                   []() { MockDaemon::instance()->deactivate(); });
  QObject::connect(&sh, &SignalHandler::quitRequested, &app,
                   &QCoreApplication::quit, Qt::QueuedConnection);

  return app.exec();
}

static Command::RegistrationProxy<MockDaemonServer> s_commandMockDaemon;
