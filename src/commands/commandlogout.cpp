/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandlogout.h"
#include "commandlineparser.h"
#include "localizer.h"
#include "models/device.h"
#include "mozillavpn.h"
#include "tasks/removedevice/taskremovedevice.h"

#include <QEventLoop>
#include <QTextStream>

CommandLogout::CommandLogout() : Command("logout", "Logout the current user.") {}

int CommandLogout::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());
    return runCommandLineApp([&]() {
        if (tokens.length() > 1) {
            QList<CommandLineParser::Option *> options;
            return CommandLineParser::unknownOption(tokens[1], tokens[0], options, false);
        }

        if (!userAuthenticated()) {
            return 1;
        }

        MozillaVPN vpn;

        if (!loadModels()) {
            QTextStream stream(stdout);
            stream << "No cache available" << Qt::endl;
            return 1;
        }

        QString deviceName = Device::currentDeviceName();
        if (vpn.deviceModel()->hasDevice(deviceName)) {
            TaskRemoveDevice *task = new TaskRemoveDevice(deviceName);
            task->run(&vpn);

            QEventLoop loop;
            QObject::connect(task, &Task::completed, [&] { loop.exit(); });
            loop.exec();
        } else {
            vpn.reset();
        }

        return 0;
    });
}

static CommandLogout s_commandLogout;
