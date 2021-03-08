/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandservers.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include "tasks/accountandservers/taskaccountandservers.h"

#include <QEventLoop>
#include <QTextStream>

CommandServers::CommandServers(QObject* parent)
    : Command(parent, "servers", "Show the list of servers.") {
  MVPN_COUNT_CTOR(CommandServers);
}

CommandServers::~CommandServers() { MVPN_COUNT_DTOR(CommandServers); }

int CommandServers::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    QString appName = tokens[0];

    CommandLineParser::Option hOption = CommandLineParser::helpOption();
    CommandLineParser::Option verboseOption("v", "verbose", "Verbose mode.");
    CommandLineParser::Option cacheOption("c", "cache", "From local cache.");

    QList<CommandLineParser::Option*> options;
    options.append(&hOption);
    options.append(&verboseOption);
    options.append(&cacheOption);

    CommandLineParser clp;
    if (clp.parse(tokens, options, false)) {
      return 1;
    }

    if (!tokens.isEmpty()) {
      return clp.unknownOption(this, appName, tokens[0], options, false);
    }

    if (hOption.m_set) {
      clp.showHelp(this, appName, options, false, false);
      return 0;
    }

    if (!userAuthenticated()) {
      return 1;
    }

    MozillaVPN vpn;

    if (!cacheOption.m_set) {
      TaskAccountAndServers task;
      task.run(&vpn);

      QEventLoop loop;
      QObject::connect(&task, &Task::completed, [&] { loop.exit(); });
      loop.exec();
    } else if (!loadModels()) {
      return 0;
    }

    QTextStream stream(stdout);

    ServerCountryModel* scm = vpn.serverCountryModel();
    for (const ServerCountry& country : scm->countries()) {
      stream << "- Country: " << country.name() << " (code: " << country.code()
             << ")" << Qt::endl;
      for (const ServerCity& city : country.cities()) {
        stream << "  - City: " << city.name() << " (" << city.code() << ")"
               << Qt::endl;
        for (const Server& server : city.servers()) {
          stream << "    - Server: " << server.hostname() << Qt::endl;

          if (verboseOption.m_set) {
            stream << "        ipv4 addr-in: " << server.ipv4AddrIn()
                   << Qt::endl;
            stream << "        ipv4 gateway: " << server.ipv4Gateway()
                   << Qt::endl;
            stream << "        ipv6 addr-in: " << server.ipv6AddrIn()
                   << Qt::endl;
            stream << "        ipv6 gateway: " << server.ipv6Gateway()
                   << Qt::endl;
            stream << "        public key: " << server.publicKey() << Qt::endl;
          }
        }
      }
    }

    return 0;
  });
}

static Command::RegistrationProxy<CommandServers> s_commandServers;
