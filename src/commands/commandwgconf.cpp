/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandwgconf.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include "commandlineparser.h"
#include "controller.h"
#include "dnshelper.h"
#include "interfaceconfig.h"
#include "leakdetector.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "mozillavpn.h"

CommandWgConf::CommandWgConf(QObject* parent)
    : Command(parent, "wgconf", "Generate a wireguard configuration file.") {
  MZ_COUNT_CTOR(CommandWgConf);
}

CommandWgConf::~CommandWgConf() { MZ_COUNT_DTOR(CommandWgConf); }

int CommandWgConf::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return MozillaVPN::runCommandLineApp([&]() {
    Q_ASSERT(!tokens.isEmpty());
    if (tokens.length() > 1) {
      QList<CommandLineParser::Option*> options;
      return CommandLineParser::unknownOption(this, tokens[1], tokens[0],
                                              options, false);
    }

    MozillaVPN vpn;
    QTextStream stream(stdout);
    if (!vpn.hasToken()) {
      stream << "User is not authenticated" << Qt::endl;
      return 1;
    }

    if (!vpn.loadModels()) {
      QTextStream stream(stdout);
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    InterfaceConfig entryConfig;
    InterfaceConfig exitConfig;
    DeviceModel* dm = vpn.deviceModel();
    Q_ASSERT(dm);
    if (!dm->hasCurrentDevice(vpn.keys())) {
      stream << "Device is not registered" << Qt::endl;
      return 1;
    }
    const Device* cd = dm->currentDevice(vpn.keys());

    ServerData* sd = vpn.serverData();
    Q_ASSERT(sd);
    // Now we need to select a server.
    Server exitServer = Server::weightChooser(sd->exitServers());
    exitConfig.m_serverIpv4Gateway = exitServer.ipv4Gateway();
    exitConfig.m_serverIpv6Gateway = exitServer.ipv6Gateway();
    exitConfig.m_serverPublicKey = exitServer.publicKey();
    exitConfig.m_serverIpv4AddrIn = exitServer.ipv4AddrIn();
    exitConfig.m_serverIpv6AddrIn = exitServer.ipv6AddrIn();
    if (sd->multihop()) {
      exitConfig.m_hopType = InterfaceConfig::MultiHopExit;
      exitConfig.m_serverPort = exitServer.multihopPort();

      // Configure entry server
      Server entryServer = Server::weightChooser(sd->entryServers());
      entryConfig.m_hopType = InterfaceConfig::MultiHopEntry;
      entryConfig.m_privateKey = vpn.keys()->privateKey();
      entryConfig.m_deviceIpv4Address = cd->ipv4Address();
      entryConfig.m_deviceIpv6Address = cd->ipv6Address();
      entryConfig.m_serverPublicKey = entryServer.publicKey();
      entryConfig.m_serverIpv4AddrIn = entryServer.ipv4AddrIn();
      entryConfig.m_serverIpv6AddrIn = entryServer.ipv6AddrIn();
      entryConfig.m_serverPort = entryServer.choosePort();
      entryConfig.m_allowedIPAddressRanges.append(
          IPAddress(exitServer.ipv4AddrIn()));
      if (!exitServer.ipv6AddrIn().isEmpty()) {
        entryConfig.m_allowedIPAddressRanges.append(
            IPAddress(exitServer.ipv6AddrIn()));
      }
    } else {
      exitConfig.m_hopType = InterfaceConfig::SingleHop;
      exitConfig.m_privateKey = vpn.keys()->privateKey();
      exitConfig.m_deviceIpv4Address = cd->ipv4Address();
      exitConfig.m_deviceIpv6Address = cd->ipv6Address();
      exitConfig.m_serverPort = exitServer.choosePort();
    }
    exitConfig.m_dnsServer = DNSHelper::getDNS(exitServer.ipv4Gateway());
    exitConfig.m_allowedIPAddressRanges =
        Controller::getAllowedIPAddressRanges(exitServer);

    // Stream it out to the user.
    if (sd->multihop()) {
      stream << entryConfig.toMultiHopWgConf(exitConfig) << Qt::endl;
    } else {
      stream << exitConfig.toWgConf() << Qt::endl;
    }
    return 0;
  });
}

static Command::RegistrationProxy<CommandWgConf> s_commandWgConf;
