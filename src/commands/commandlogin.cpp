/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogin.h"
#include "commandlineparser.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"
#include "tasks/authenticate/taskauthenticate.h"

#include <QEventLoop>
#include <QTextStream>

CommandLogin::CommandLogin() : Command("login", "Starts the authentication flow.") {}

int CommandLogin::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());

    if (tokens.length() > 1) {
        QList<CommandLineParser::Option *> options;
        return CommandLineParser::unknownOption(tokens[1], tokens[0], options, false);
    }

    if (SettingsHolder::instance()->hasToken()) {
        QTextStream stream(stdout);
        stream << "User status: already authenticated" << Qt::endl;
        return 1;
    }

    SimpleNetworkManager snm;

    MozillaVPN vpn;

    TaskAuthenticate *task = new TaskAuthenticate();
    task->run(&vpn);

    QEventLoop loop;
    QObject::connect(task, &Task::completed, [&] { loop.exit(); });
    loop.exec();

    return 0;
}

static CommandLogin s_commandLogin;
