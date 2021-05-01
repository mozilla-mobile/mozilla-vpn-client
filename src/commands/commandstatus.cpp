/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandstatus.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"
#include "tasks/accountandservers/taskaccountandservers.h"

#include <QEventLoop>
#include <QTextStream>

CommandStatus::CommandStatus(QObject* parent)
    : Command(parent, "status", "Show the current VPN status.") {
  MVPN_COUNT_CTOR(CommandStatus);
}

CommandStatus::~CommandStatus() { MVPN_COUNT_DTOR(CommandStatus); }

int CommandStatus::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
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

    if (!userAuthenticated()) {
      return 0;
    }

    QTextStream stream(stdout);
    stream << "User status: authenticated" << Qt::endl;

    if (!loadModels()) {
      return 1;
    }

    if (!cacheOption.m_set) {
      TaskAccountAndServers task;
      task.run(&vpn);

      QEventLoop loop;
      QObject::connect(&task, &Task::completed, [&] { loop.exit(); });
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

    ServerData* sd = vpn.currentServer();
    if (sd) {
      stream << "Server country code: " << sd->countryCode() << Qt::endl;
      stream << "Server country: " << sd->countryName() << Qt::endl;
      stream << "Server city: " << sd->cityName() << Qt::endl;
    }

    return 0;
  });
}

static Command::RegistrationProxy<CommandStatus> s_commandStatus;
