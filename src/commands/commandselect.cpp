/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandselect.h"
#include "mozillavpn.h"
#include "simplenetworkmanager.h"

#include <QTextStream>

CommandSelect::CommandSelect() : Command("select", "Select a server") {}

int CommandSelect::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());
    QString app = tokens[0];

    if (tokens.length() != 2) {
        QTextStream stream(stdout);
        stream << "usage: " << app << " <server_hostname>" << Qt::endl;
        stream << Qt::endl;
        stream << "The list of <server_hostname>s can be obtained using: '" << app << " servers'"
               << Qt::endl;
        return 1;
    }

    if (!userAuthenticated()) {
        return 1;
    }

    SimpleNetworkManager snm;

    MozillaVPN vpn;
    if (!loadModels()) {
        return 1;
    }

    ServerData sd;
    if (!pickServer(tokens[1], sd)) {
        QTextStream stream(stdout);
        stream << "unknown server hostname: " << tokens[1] << Qt::endl;
        return 1;
    }

    vpn.changeServer(sd.countryCode(), sd.city());
    return 0;
}

bool CommandSelect::pickServer(const QString &hostname, ServerData &serverData)
{
    for (const ServerCountry &country : MozillaVPN::instance()->serverCountryModel()->countries()) {
        for (const ServerCity &city : country.cities()) {
            for (const Server &server : city.getServers()) {
                if (server.hostname() == hostname) {
                    serverData.update(country.code(), country.name(), city.name());
                    return true;
                }
            }
        }
    }

    return false;
}

static CommandSelect s_commandSelect;
