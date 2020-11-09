/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandstatus.h"
#include "commandlineparser.h"
#include "settingsholder.h"

#include <QTextStream>

CommandStatus::CommandStatus() : Command("status", "Show the current VPN status.") {}

int CommandStatus::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());

    if (tokens.length() > 1) {
        QList<CommandLineParser::Option *> options;
        CommandLineParser::unknownOption(tokens[1], tokens[0], options, false);
        Q_UNREACHABLE();
    }

    if (!SettingsHolder::instance()->hasToken()) {
        QTextStream stream(stdout);
        stream << "User status: not authenticated" << Qt::endl;
        return 0;
    }

    QTextStream stream(stdout);
    stream << "User status: authenticated" << Qt::endl;

    return 0;
}

static CommandStatus s_commandStatus;
