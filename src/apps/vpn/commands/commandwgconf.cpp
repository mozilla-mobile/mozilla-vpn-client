/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandwgconf.h"

#include <QEventLoop>
#include <QTextStream>

#include "commandlineparser.h"
#include "controller.h"
#include "dnshelper.h"
#include "interfaceconfig.h"
#include "leakdetector.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/serverdata.h"
#include "mozillavpn.h"
#include "settingsholder.h"

CommandWgConf::CommandWgConf(QObject* parent)
    : Command(parent, "wgconf", "Generate a wireguard configuration file.") {
  MZ_COUNT_CTOR(CommandWgConf);
}

CommandWgConf::~CommandWgConf() { MZ_COUNT_DTOR(CommandWgConf); }

int CommandWgConf::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    Q_ASSERT(!tokens.isEmpty());
    if (tokens.length() > 1) {
      QList<CommandLineParser::Option*> options;
      return CommandLineParser::unknownOption(this, tokens[1], tokens[0],
                                              options, false);
    }

    MozillaVPN vpn;
    QTextStream stream(stdout);

    if (!userAuthenticated()) {
      stream << "User is not authenticated" << Qt::endl;
      return 1;
    }

    if (!loadModels()) {
      return 1;
    }

    InterfaceConfig config;
    DeviceModel* dm = vpn.deviceModel();
    Q_ASSERT(dm);
    if (!dm->hasCurrentDevice(vpn.keys())) {
      stream << "Device is not registered" << Qt::endl;
      return 1;
    }
    const Device* cd = dm->currentDevice(vpn.keys());
    config.m_hopType = InterfaceConfig::SingleHop;
    config.m_privateKey = vpn.keys()->privateKey();
    config.m_deviceIpv4Address = cd->ipv4Address();
    config.m_deviceIpv6Address = cd->ipv6Address();

    ServerData* sd = vpn.serverData();
    Q_ASSERT(sd);
    // Now we need to select a server.
    Server exitServer = Server::weightChooser(sd->exitServers());
    config.m_serverIpv4Gateway = exitServer.ipv4Gateway();
    config.m_serverIpv6Gateway = exitServer.ipv6Gateway();
    config.m_serverPublicKey = exitServer.publicKey();
    config.m_serverIpv4AddrIn = exitServer.ipv4AddrIn();
    config.m_serverIpv6AddrIn = exitServer.ipv6AddrIn();
    if (sd->multihop()) {
      Server entryServer = Server::weightChooser(sd->entryServers());
      config.m_serverPort = entryServer.multihopPort();
    } else {
      config.m_serverPort = exitServer.choosePort();
    }
    config.m_dnsServer = DNSHelper::getDNS(exitServer.ipv4Gateway());
    config.m_allowedIPAddressRanges =
    ConnectionManager::getAllowedIPAddressRanges(exitServer);

    // Stream it out to the user.
    stream << config.toWgConf() << Qt::endl;
    return 0;
  });
}

static Command::RegistrationProxy<CommandWgConf> s_commandWgConf;
