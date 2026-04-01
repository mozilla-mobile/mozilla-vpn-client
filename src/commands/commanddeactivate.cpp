/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commanddeactivate.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "constants.h"
#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

CommandDeactivate::CommandDeactivate(QObject* parent)
    : Command(parent, "deactivate", "Deactivate the VPN tunnel") {
  MZ_COUNT_CTOR(CommandDeactivate);
}

CommandDeactivate::~CommandDeactivate() { MZ_COUNT_DTOR(CommandDeactivate); }

int CommandDeactivate::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option testingOption("t", "testing",
                                          "Run in testing mode.");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&testingOption);

  CommandLineParser clp;
  if (clp.parse(tokens, options, false)) {
    return 1;
  }

  if (hOption.m_set) {
    clp.showHelp(this, appName, options, false, false);
    return 0;
  }

  if (testingOption.m_set) {
    QCoreApplication::setOrganizationName("Mozilla Testing");
    LogHandler::instance()->setStderr(true);
  }

  return MozillaVPN::runCommandLineApp([&]() {
    MozillaVPN vpn;
    if (testingOption.m_set) {
      Constants::setStaging();
    }
    if (!vpn.hasToken()) {
      QTextStream stream(stdout);
      stream << "User status: not authenticated" << Qt::endl;
      return 1;
    }
    if (!vpn.loadModels()) {
      QTextStream stream(stdout);
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    Controller controller;

    QEventLoop loop;
    QObject::connect(&controller, &Controller::stateChanged, &vpn, [&] {
      if (controller.state() == Controller::StateOff ||
          controller.state() == Controller::StateOn) {
        loop.exit();
      }
    });
    controller.initialize();
    loop.exec();
    controller.disconnect();

    // If we are connecting right now, we want to wait untile the operation is
    // completed.
    if (controller.state() != Controller::StateOff &&
        controller.state() != Controller::StateOn) {
      QObject::connect(&controller, &Controller::stateChanged, &vpn, [&] {
        if (controller.state() == Controller::StateOff ||
            controller.state() == Controller::StateOn) {
          loop.exit();
        }
      });
      loop.exec();
      controller.disconnect();
    }

    if (controller.state() == Controller::StateOff) {
      QTextStream stream(stdout);
      stream << "The VPN tunnel is already inactive" << Qt::endl;
      return 0;
    }

    QObject::connect(&controller, &Controller::stateChanged, &vpn, [&] {
      if (controller.state() == Controller::StateOff ||
          controller.state() == Controller::StateOn) {
        loop.exit();
      }
    });

    controller.deactivate();
    loop.exec();
    controller.disconnect();

    if (controller.state() == Controller::StateOff) {
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
