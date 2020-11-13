/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogin.h"
#include "commandlineparser.h"
#include "localizer.h"
#include "models/devicemodel.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "tasks/authenticate/taskauthenticate.h"

#include <QEventLoop>
#include <QTextStream>

CommandLogin::CommandLogin() : Command("login", "Starts the authentication flow.") {}

int CommandLogin::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());
    return runGuiApp([&]() {
        if (tokens.length() > 1) {
            QList<CommandLineParser::Option *> options;
            return CommandLineParser::unknownOption(tokens[1], tokens[0], options, false);
        }

        if (SettingsHolder::instance()->hasToken()) {
            QTextStream stream(stdout);
            stream << "User status: already authenticated" << Qt::endl;
            return 1;
        }

        MozillaVPN vpn;
        vpn.authenticate();

        QEventLoop loop;
        QObject::connect(&vpn, &MozillaVPN::stateChanged, [&] {
            if (vpn.state() == MozillaVPN::StatePostAuthentication || vpn.state() == MozillaVPN::StateMain) {
                loop.exit();
            }
        });
        loop.exec();

        QString deviceName = Device::currentDeviceName();
        if (!vpn.deviceModel()->hasDevice(deviceName)) {
            QTextStream stream(stdout);
            stream << "Device limit reached" << Qt::endl;
            return 1;
        }

        return 0;
    });
}

static CommandLogin s_commandLogin;
