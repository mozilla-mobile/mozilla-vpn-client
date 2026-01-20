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
    QString appName = tokens[0];

    CommandLineParser::Option hOption = CommandLineParser::helpOption();
    CommandLineParser::Option mullvaldMultihop(
        "m", "mullvald-multihop", "Generate config for Mullvad multihop.");
    CommandLineParser::Option wireguardMultihop(
        "w", "wireguard-multihop", "Generate config for Wireguard multihop.");

    QList<CommandLineParser::Option*> options;
    options.append(&hOption);
    options.append(&mullvaldMultihop);
    options.append(&wireguardMultihop);

    CommandLineParser clp;
    if (clp.parse(tokens, options, false)) {
      return 1;
    }

    if (hOption.m_set) {
      clp.showHelp(this, appName, options, false, false);
      return 0;
    }

    QTextStream stream(stdout);
    if (mullvaldMultihop.m_set && wireguardMultihop.m_set) {
      stream << "Cannot use both --mullvald-multihop and --wireguard-multihop"
             << Qt::endl;
      return 1;
    }

    MozillaVPN vpn;
    if (!vpn.hasToken()) {
      stream << "User is not authenticated" << Qt::endl;
      return 1;
    }

    if (!vpn.loadModels()) {
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    const bool useWireguardMultihop =
#if defined(MZ_LINUX)
        !mullvaldMultihop.m_set;
#else
        wireguardMultihop.m_set;
#endif

    InterfaceConfig entryConfig;
    InterfaceConfig exitConfig;
    QString entryPeerComment;
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
    exitConfig.m_hopType = InterfaceConfig::SingleHop;
    exitConfig.m_privateKey = vpn.keys()->privateKey();
    exitConfig.m_deviceIpv4Address = cd->ipv4Address();
    exitConfig.m_deviceIpv6Address = cd->ipv6Address();
    exitConfig.m_serverIpv4Gateway = exitServer.ipv4Gateway();
    exitConfig.m_serverIpv6Gateway = exitServer.ipv6Gateway();
    exitConfig.m_serverPublicKey = exitServer.publicKey();
    exitConfig.m_serverIpv4AddrIn = exitServer.ipv4AddrIn();
    exitConfig.m_serverIpv6AddrIn = exitServer.ipv6AddrIn();
    exitConfig.m_serverPort = exitServer.choosePort();
    QString exitPeerComment = QString("Exit Server: %1 - %2 (%3)")
                                  .arg(exitServer.hostname())
                                  .arg(sd->localizedExitCityName())
                                  .arg(sd->exitCountryCode());
    if (sd->multihop()) {
      // Configure entry server
      Server entryServer = Server::weightChooser(sd->entryServers());
      entryPeerComment = QString("Entry Server: %1 - %2 (%3)")
                             .arg(entryServer.hostname())
                             .arg(sd->localizedEntryCityName())
                             .arg(sd->entryCountryCode());
      if (useWireguardMultihop) {
        exitConfig.m_hopType = InterfaceConfig::MultiHopExit;
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
        // configure main peer for Mullvald multihop
        exitPeerComment = entryPeerComment + "\n" + exitPeerComment;
        exitConfig.m_serverIpv4AddrIn = entryServer.ipv4AddrIn();
        exitConfig.m_serverIpv6AddrIn = entryServer.ipv6AddrIn();
        exitConfig.m_serverPort = exitServer.multihopPort();
      }
    }
    exitConfig.m_dnsServer = DNSHelper::getDNS(exitServer.ipv4Gateway());
    exitConfig.m_allowedIPAddressRanges =
        Controller::getAllowedIPAddressRanges(exitServer);

    // Stream it out to the user.
    if (sd->multihop() && useWireguardMultihop) {
      stream << "# This file contains configuration settings for a multi-hop "
                "WireGuard setup\n";
      stream << "# The configuration is intended for Linux systems and has "
                "been tested to work\n";
      stream << "# with both wg-quick and NetworkManager-managed WireGuard "
                "connections.\n";
      stream << "# IMPORTANT:\n";
      stream << "# For a proper multi-hop setup, the client must configure "
                "specific routes so\n";
      stream << "# that traffic is correctly forwarded through each WireGuard "
                "hop. If these\n";
      stream << "# routes are missing or incorrect, the configuration may "
                "silently degrade into\n";
      stream << "# a single-hop setup.\n";
      stream << "# If you are unsure how to check the required routes, you can "
                "generate a\n";
      stream << "# configuration that works across all supported systems by "
                "using the\n";
      stream << "# --mullvald-multihop flag.\n\n";
      stream << entryConfig.toMultiHopWgConf(exitConfig,
                                             QMap<QString, QString>(),
                                             exitPeerComment, entryPeerComment)
             << Qt::endl;
    } else {
      if (sd->multihop()) {
        stream << "# This file contains configuration settings for a multi-hop "
                  "WireGuard setup\n";
        stream << "# It utilizes Mullvad's Multi-Hop feature, which routes "
                  "traffic through multiple\n";
        stream << "# WireGuard nodes. In this setup, data is sent to the entry "
                  "server using a\n";
        stream << "# specific port and the exit server's public key, allowing "
                  "the entry server to\n";
        stream << "# forward the traffic to the exit server.\n";
        stream << "# For more information see: "
                  "https://mullvad.net/en/help/wireguard-multi-hop/\n\n";
      }
      stream << exitConfig.toWgConf(QMap<QString, QString>(), exitPeerComment)
             << Qt::endl;
    }
    return 0;
  });
}

static Command::RegistrationProxy<CommandWgConf> s_commandWgConf;
