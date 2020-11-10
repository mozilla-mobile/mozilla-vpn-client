/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandservers.h"
#include "commandlineparser.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"
#include "tasks/accountandservers/taskaccountandservers.h"

#include <QEventLoop>
#include <QTextStream>

CommandServers::CommandServers() : Command("servers", "Show the list of servers.") {}

int CommandServers::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());

    CommandLineParser clp;

    QList<CommandLineParser::Option *> options;

    CommandLineParser::Option verboseOption("v", "verbose", "Verbose");
    options.append(&verboseOption);

    clp.parse(tokens, options, false);

    if (!tokens.isEmpty()) {
        clp.unknownOption("ui", tokens[0], options, false);
        Q_UNREACHABLE();
    }

    if (!SettingsHolder::instance()->hasToken()) {
        QTextStream stream(stdout);
        stream << "User status: not authenticated" << Qt::endl;
        return 0;
    }

    SimpleNetworkManager snm;

    MozillaVPN vpn;
    TaskAccountAndServers *task = new TaskAccountAndServers();
    task->run(&vpn);

    QEventLoop loop;
    QObject::connect(task, &Task::completed, [&] { loop.exit(); });
    loop.exec();

    QTextStream stream(stdout);

    ServerCountryModel *scm = vpn.serverCountryModel();
    for (const ServerCountry &country : scm->countries()) {
        stream << "- Country: " << country.name() << " (code: " << country.code() << ")"
               << Qt::endl;
        for (const ServerCity &city : country.cities()) {
            stream << "  - City: " << city.name() << " (" << city.code() << ")" << Qt::endl;
            if (verboseOption.m_set) {
                for (const Server &server : city.getServers()) {
                    stream << "    - Server:" << Qt::endl;
                    stream << "        hostname: " << server.hostname() << Qt::endl;
                    stream << "        ipv4 addr-in: " << server.ipv4AddrIn() << Qt::endl;
                    stream << "        ipv4 gateway: " << server.ipv4Gateway() << Qt::endl;
                    stream << "        ipv6 addr-in: " << server.ipv6AddrIn() << Qt::endl;
                    stream << "        ipv6 gateway: " << server.ipv6Gateway() << Qt::endl;
                    stream << "        public key: " << server.publicKey() << Qt::endl;
                }
            }
        }
    }

    return 0;
}

static CommandServers s_commandServers;
