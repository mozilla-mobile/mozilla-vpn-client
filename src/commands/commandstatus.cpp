/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandstatus.h"

#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "controller.h"
#include "leakdetector.h"
#include "models/devicemodel.h"
#include "models/servercountrymodel.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "tasks/account/taskaccount.h"

CommandStatus::CommandStatus(QObject* parent)
    : Command(parent, "status", "Show the current VPN status.") {
  MZ_COUNT_CTOR(CommandStatus);
}

CommandStatus::~CommandStatus() { MZ_COUNT_DTOR(CommandStatus); }

int CommandStatus::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return MozillaVPN::runCommandLineApp([&]() {
    QString appName = tokens[0];

    CommandLineParser::Option hOption = CommandLineParser::helpOption();
    CommandLineParser::Option cacheOption("c", "cache", "From local cache.");

    QList<CommandLineParser::Option*> options;
    options.append(&hOption);
    options.append(&cacheOption);

    CommandLineParser clp;
    if (clp.parse(tokens, options, false)) {
      return 1;
    }

    if (!tokens.isEmpty()) {
      QList<CommandLineParser::Option*> options;
      return CommandLineParser::unknownOption(this, tokens[1], tokens[0],
                                              options, false);
    }

    if (hOption.m_set) {
      clp.showHelp(this, appName, options, false, false);
      return 0;
    }

    MozillaVPN vpn;
    QTextStream stream(stdout);
    if (!vpn.hasToken()) {
      stream << "User status: not authenticated" << Qt::endl;
      return 0;
    }
    stream << "User status: authenticated" << Qt::endl;

    if (!vpn.loadModels()) {
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    if (!cacheOption.m_set) {
      TaskAccount task(ErrorHandler::PropagateError);
      task.run();

      QEventLoop loop;
      QObject::connect(&task, &Task::completed, &task, [&] { loop.exit(); });
      loop.exec();
    }

    User* user = vpn.user();
    Q_ASSERT(user);
    stream << "User avatar: " << user->avatar() << Qt::endl;
    stream << "User displayName: " << user->displayName() << Qt::endl;
    stream << "User email: " << user->email() << Qt::endl;
    stream << "User maxDevices: " << user->maxDevices() << Qt::endl;
    stream << "User subscription needed: "
           << (user->subscriptionNeeded() ? "true" : "false") << Qt::endl;

    DeviceModel* dm = vpn.deviceModel();
    Q_ASSERT(dm);
    stream << "Active devices: " << dm->activeDevices() << Qt::endl;

    const Device* cd = dm->currentDevice(vpn.keys());
    if (cd) {
      stream << "Current devices:" << cd->name() << Qt::endl;
    }

    const QList<Device>& devices = dm->devices();
    for (int i = 0; i < devices.length(); ++i) {
      const Device& device = devices.at(i);
      stream << "Device " << (i + 1) << Qt::endl;
      stream << " - name: " << device.name() << Qt::endl;
      stream << " - creation time: " << device.createdAt().toString()
             << Qt::endl;
      stream << " - public key: " << device.publicKey() << Qt::endl;
      stream << " - ipv4 address: " << device.ipv4Address() << Qt::endl;
      stream << " - ipv6 address: " << device.ipv6Address() << Qt::endl;
    }

    ServerCountryModel* model = vpn.serverCountryModel();
    ServerData* sd = vpn.serverData();
    Q_ASSERT(sd);

    stream << "Server country code: " << sd->exitCountryCode() << Qt::endl;
    stream << "Server country: " << model->countryName(sd->exitCountryCode())
           << Qt::endl;
    stream << "Server city: " << sd->exitCityName() << Qt::endl;

    Controller controller;

    QEventLoop loop;
    QObject::connect(&controller, &Controller::stateChanged, &controller, [&] {
      if (controller.state() == Controller::StateOff ||
          controller.state() == Controller::StateOn) {
        loop.exit();
      }
    });
    controller.initialize();
    loop.exec();

    stream << "VPN state: ";
    switch (controller.state()) {
      case Controller::StateInitializing:
        stream << "initializing";
        break;

      case Controller::StatePermissionRequired:
        stream << "permission-required";
        break;

      case Controller::StateOff:
        stream << "off";
        break;
      case Controller::StateConnecting:
        stream << "connecting";
        break;

      case Controller::StateConfirming:
        stream << "confirming";
        break;

      case Controller::StateOn:
        [[fallthrough]];
      case Controller::StateOnPartial:
        [[fallthrough]];
      case Controller::StateSilentSwitching:
        stream << "on";
        break;

      case Controller::StateDisconnecting:
        stream << "disconnecting";
        break;

      case Controller::StateSwitching:
        stream << "switching";
        break;
    }

    stream << Qt::endl;

    return 0;
  });
}

static Command::RegistrationProxy<CommandStatus> s_commandStatus;
