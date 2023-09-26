/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commanddeactivate.h"

#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "controller.h"
#include "leakdetector.h"
#include "mozillavpn.h"

CommandDeactivate::CommandDeactivate(QObject* parent)
    : Command(parent, "deactivate", "Deactivate the VPN tunnel") {
  MZ_COUNT_CTOR(CommandDeactivate);
}

CommandDeactivate::~CommandDeactivate() { MZ_COUNT_DTOR(CommandDeactivate); }

int CommandDeactivate::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    if (tokens.length() > 1) {
      QList<CommandLineParser::Option*> options;
      return CommandLineParser::unknownOption(this, tokens[1], tokens[0],
                                              options, false);
    }

    if (!userAuthenticated()) {
      return 1;
    }

    MozillaVPN vpn;
    if (!loadModels()) {
      return 1;
    }

    ConnectionManager connectionManager;

    QEventLoop loop;
    QObject::connect(
        &connectionManager, &ConnectionManager::stateChanged, &vpn, [&] {
          if (connectionManager.state() == ConnectionManager::StateOff ||
              connectionManager.state() == ConnectionManager::StateIdle) {
            loop.exit();
          }
        });
    connectionManager.initialize();
    loop.exec();
    connectionManager.disconnect();

    // If we are connecting right now, we want to wait untile the operation is
    // completed.
    if (connectionManager.state() != ConnectionManager::StateOff &&
        connectionManager.state() != ConnectionManager::StateIdle) {
      QObject::connect(
          &connectionManager, &ConnectionManager::stateChanged, &vpn, [&] {
            if (connectionManager.state() == ConnectionManager::StateOff ||
                connectionManager.state() == ConnectionManager::StateIdle) {
              loop.exit();
            }
          });
      loop.exec();
      connectionManager.disconnect();
    }

    if (connectionManager.state() == ConnectionManager::StateOff) {
      QTextStream stream(stdout);
      stream << "The VPN tunnel is already inactive" << Qt::endl;
      return 0;
    }

    QObject::connect(
        &connectionManager, &ConnectionManager::stateChanged, &vpn, [&] {
          if (connectionManager.state() == ConnectionManager::StateOff ||
              connectionManager.state() == ConnectionManager::StateIdle) {
            loop.exit();
          }
        });

    connectionManager.deactivate();
    loop.exec();
    connectionManager.disconnect();

    if (connectionManager.state() == ConnectionManager::StateOff) {
      QTextStream stream(stdout);
      stream << "The VPN tunnel is now inactive" << Qt::endl;
      return 0;
    }

    QTextStream stream(stdout);
    stream << "The VPN tunnel deactivation failed" << Qt::endl;
    return 1;
  });
}

static Command::RegistrationProxy<CommandDeactivate> s_commandDeactivate;
