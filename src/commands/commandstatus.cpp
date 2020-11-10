/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandstatus.h"
#include "commandlineparser.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"
#include "tasks/accountandservers/taskaccountandservers.h"

#include <QEventLoop>
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

    SimpleNetworkManager snm;

    MozillaVPN vpn;
    TaskAccountAndServers* task = new TaskAccountAndServers();
    task->run(&vpn);

    QEventLoop loop;
    QObject::connect(task, &Task::completed, [&] { loop.exit(); });
    loop.exec();

    User *user = vpn.user();
    Q_ASSERT(user);
    stream << "User avatar: " << user->avatar() << Qt::endl;
    stream << "User displayName: " << user->displayName() << Qt::endl;
    stream << "User email: " << user->email() << Qt::endl;
    stream << "User maxDevices: " << user->maxDevices() << Qt::endl;
    stream << "User subscription needed: " << (user->subscriptionNeeded() ? "true" : "false")
           << Qt::endl;

    DeviceModel *dm = vpn.deviceModel();
    Q_ASSERT(dm);
    stream << "Active devices: " << dm->activeDevices() << Qt::endl;

    const Device *cd = dm->currentDevice();
    if (cd) {
        stream << "Current devices:" << cd->name() << Qt::endl;
    }

    const QList<Device> &devices = dm->devices();
    for (int i = 0; i < devices.length(); ++i) {
        const Device &device = devices.at(i);
        stream << "Device " << i << " name: " << device.name() << Qt::endl;
        stream << "Device " << i << " creation time: " << device.createdAt().toString() << Qt::endl;
        stream << "Device " << i << " public key: " << device.publicKey() << Qt::endl;
        stream << "Device " << i << " ipv4 address: " << device.ipv4Address() << Qt::endl;
        stream << "Device " << i << " ipv6 address: " << device.ipv6Address() << Qt::endl;
    }

    ServerData *sd = vpn.currentServer();
    if (sd) {
        stream << "Server country code: " << sd->countryCode() << Qt::endl;
        stream << "Server country: " << sd->country() << Qt::endl;
        stream << "Server city: " << sd->city() << Qt::endl;
    }

    return 0;
}

static CommandStatus s_commandStatus;
