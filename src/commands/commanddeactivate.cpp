/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commanddeactivate.h"
#include "commandlineparser.h"
#include "mozillavpn.h"
#include "simplenetworkmanager.h"

#include <QEventLoop>
#include <QTextStream>

CommandDeactivate::CommandDeactivate() : Command("deactivate", "Deactivate the VPN tunnel") {}

int CommandDeactivate::run(QStringList &tokens)
{
    Q_ASSERT(!tokens.isEmpty());

    if (tokens.length() > 1) {
        QList<CommandLineParser::Option *> options;
        return CommandLineParser::unknownOption(tokens[1], tokens[0], options, false);
    }

    if (!userAuthenticated()) {
        return 1;
    }

    SimpleNetworkManager snm;

    MozillaVPN vpn;
    if (!vpn.deviceModel()->fromSettings() || !vpn.serverCountryModel()->fromSettings()
        || !vpn.user()->fromSettings() || !vpn.keys()->fromSettings()
        || !vpn.currentServer()->fromSettings()) {
        QTextStream stream(stdout);
        stream << "No cache available" << Qt::endl;
        return 1;
    }

    if (!vpn.modelsInitialized()) {
        QTextStream stream(stdout);
        stream << "Something went wrong when initializing the models" << Qt::endl;
        return 1;
    }

    Controller controller;

    QEventLoop loop;
    QObject::connect(&controller, &Controller::stateChanged, [&] {
        if (controller.state() == Controller::StateOff
            || controller.state() == Controller::StateOn) {
            loop.exit();
        }
    });
    controller.initialize();
    loop.exec();
    controller.disconnect();

    // If we are connecting right now, we want to wait untile the operation is completed.
    if (controller.state() != Controller::StateOff && controller.state() != Controller::StateOn) {
        QObject::connect(&controller, &Controller::stateChanged, [&] {
            if (controller.state() == Controller::StateOff
                || controller.state() == Controller::StateOn) {
                loop.exit();
            }
        });
        loop.exec();
        controller.disconnect();
    }

    if (controller.state() == Controller::StateOff) {
        QTextStream stream(stdout);
        stream << "The VPN tunnel is already inactive" << Qt::endl;
        return 0;
    }

    QObject::connect(&controller, &Controller::stateChanged, [&] {
        if (controller.state() == Controller::StateOff
            || controller.state() == Controller::StateOn) {
            loop.exit();
        }
    });

    controller.deactivate();
    loop.exec();
    controller.disconnect();

    if (controller.state() == Controller::StateOff) {
        QTextStream stream(stdout);
        stream << "The VPN tunnel is now inactive" << Qt::endl;
        return 0;
    }

    QTextStream stream(stdout);
    stream << "The VPN tunnel deactivation failed" << Qt::endl;
    return 1;
}

static CommandDeactivate s_commandDeactivte;
