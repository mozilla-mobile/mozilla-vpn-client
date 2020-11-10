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
    QString app = tokens[0];

    CommandLineParser clp;

    QList<CommandLineParser::Option *> options;

    CommandLineParser::Option hOption = CommandLineParser::helpOption();
    options.append(&hOption);

    CommandLineParser::Option verboseOption("v", "verbose", "Verbose");
    options.append(&verboseOption);

    CommandLineParser::Option cacheOption("c", "cache", "From Cache");
    options.append(&cacheOption);

    clp.parse(tokens, options, false);

    if (!tokens.isEmpty()) {
        clp.unknownOption(app, tokens[0], options, false);
        Q_UNREACHABLE();
    }

    if (hOption.m_set) {
        clp.showHelp(app, options, false, false);
        return 0;
    }

    if (!SettingsHolder::instance()->hasToken()) {
        QTextStream stream(stdout);
        stream << "User status: not authenticated" << Qt::endl;
        return 0;
    }

    SimpleNetworkManager snm;

    MozillaVPN vpn;

    if (!cacheOption.m_set) {
        TaskAccountAndServers *task = new TaskAccountAndServers();
        task->run(&vpn);

        QEventLoop loop;
        QObject::connect(task, &Task::completed, [&] { loop.exit(); });
        loop.exec();
    } else {
        if (!vpn.serverCountryModel()->fromSettings()) {
            QTextStream stream(stdout);
            stream << "No cache available" << Qt::endl;
            return 0;
        }
    }

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
