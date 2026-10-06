/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandwgconf.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QTextStream>

#include "authenticationlistener.h"
#include "commandlineparser.h"
#include "constants.h"
#include "controller.h"
#include "dnshelper.h"
#include "errorhandler.h"
#include "interfaceconfig.h"
#include "leakdetector.h"
#include "loghandler.h"
#include "models/apierror.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "task.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/authenticate/taskauthenticate.h"

namespace {

// Auth helper: runs a headless authentication and returns the token, or an
// empty string in case of failure. The login URL is printed on the console, and
// the code is read from stdin.

QString authenticate() {
  TaskAuthenticate task(
      AuthenticationListener::AuthenticationInBrowserHeadless);

  QString token;
  QEventLoop loop;

  QObject::connect(&task, &TaskAuthenticate::authenticationCompleted, &task,
                   [&](const QByteArray&, const QString& authToken) {
                     token = authToken;
                     loop.exit();
                   });
  QObject::connect(&task, &TaskAuthenticate::authenticationAborted, &task,
                   [&] { loop.exit(); });
  QObject::connect(&task, &Task::completed, &task, [&] { loop.exit(); });
  QObject::connect(
      ErrorHandler::instance(), &ErrorHandler::alertChanged, &task, [&] {
        if (ErrorHandler::instance()->alert() != ErrorHandler::NoAlert) {
          loop.exit();
        }
      });

  QObject::connect(&task, &TaskAuthenticate::authenticationStarted, &task, [&] {
    QTextStream stream(stdout);
    stream << "Enter the code:" << " ";
    stream.flush();
    task.authenticatePkceSuccess(QTextStream(stdin).readLine());
  });

  task.run();
  loop.exec();

  return token;
}

}  // anonymous namespace

CommandWgConf::CommandWgConf(QObject* parent)
    : Command(parent, "wgconf", "Generate a wireguard configuration file.") {
  MZ_COUNT_CTOR(CommandWgConf);
}

CommandWgConf::~CommandWgConf() { MZ_COUNT_DTOR(CommandWgConf); }

int CommandWgConf::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  QString appName = tokens[0];

  CommandLineParser::Option hOption = CommandLineParser::helpOption();
  CommandLineParser::Option mullvadMultihop(
      "m", "mullvad-multihop", "Generate config for Mullvad multihop.");
  CommandLineParser::Option wireguardMultihop(
      "w", "wireguard-multihop", "Generate config for Wireguard multihop.");
  CommandLineParser::Option createSharedDevice(
      "s", "shared-device",
      "Register a shared device and generate its config. It requires a new "
      "authentication, new keys will be generated and printed only once.");
  CommandLineParser::Option testingOption("t", "testing",
                                          "Run in testing mode.");

  QList<CommandLineParser::Option*> options;
  options.append(&hOption);
  options.append(&mullvadMultihop);
  options.append(&wireguardMultihop);
  options.append(&testingOption);
  options.append(&createSharedDevice);

  CommandLineParser clp;
  if (clp.parse(tokens, options, false)) {
    return 1;
  }

  if (hOption.m_set) {
    clp.showHelp(this, appName, options, false, false);
    return 0;
  }

  if (testingOption.m_set) {
    QCoreApplication::setOrganizationName("Mozilla Testing");
    LogHandler::instance()->setStderr(true);
  }

  QTextStream stream(stdout);
  if (mullvadMultihop.m_set && wireguardMultihop.m_set) {
    stream << "Cannot use both --mullvad-multihop and --wireguard-multihop"
           << Qt::endl;
    return 1;
  }

  if (tokens.length() > 1 || (!tokens.isEmpty() && !createSharedDevice.m_set)) {
    stream << "usage: " << appName << " [options] [<device_name>]" << Qt::endl;
    stream << Qt::endl;
    stream << "<device_name> can be used with --shared-device only."
           << Qt::endl;
    return 1;
  }

  return MozillaVPN::runCommandLineApp([&]() {
    MozillaVPN vpn;
    if (testingOption.m_set) {
      Constants::setStaging();
    }
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
        !mullvadMultihop.m_set;
#else
        wireguardMultihop.m_set;
#endif

    InterfaceConfig entryConfig;
    InterfaceConfig exitConfig;
    QString entryPeerComment;

    QString devicePrivateKey;
    QString deviceIpv4Address;
    QString deviceIpv6Address;
    QString newDeviceComment;

    DeviceModel* dm = vpn.deviceModel();
    Q_ASSERT(dm);
    if (!dm->hasCurrentDevice(vpn.keys())) {
      stream << "Device is not registered" << Qt::endl;
      return 1;
    }

    if (createSharedDevice.m_set) {
      QString deviceName =
          tokens.isEmpty()
              ? QString("%1 (wgconf)").arg(Device::currentDeviceName())
              : tokens[0];

      QString token = authenticate();
      if (token.isEmpty()) {
        stream << "Authentication failed" << Qt::endl;
        return 1;
      }
      QString errorMessage;

      QEventLoop loop;
      TaskAddDevice deviceTask(deviceName, QString(), true, token);
      QObject::connect(&deviceTask, &Task::completed, &deviceTask,
                       [&] { loop.exit(); });
      deviceTask.run();
      loop.exec();

      if (deviceTask.error().errnum() != ApiError::NoError) {
        stream << "Failed to add the device: " << deviceTask.error().message()
               << Qt::endl;
        return 1;
      }

      devicePrivateKey = deviceTask.privateKey();
      deviceIpv4Address = deviceTask.device().ipv4Address();
      deviceIpv6Address = deviceTask.device().ipv6Address();

      newDeviceComment =
          QString(
              "# A new device has been registered: %1\n"
              "# Neither the device nor its keys are stored by this client.\n"
              "# Public key: %2\n"
              "# Private key: %3\n\n")
              .arg(deviceName, deviceTask.publicKey(), devicePrivateKey);
    } else {
      const Device* cd = dm->currentDevice(vpn.keys());

      devicePrivateKey = vpn.keys()->privateKey();
      deviceIpv4Address = cd->ipv4Address();
      deviceIpv6Address = cd->ipv6Address();
    }

    ServerData* sd = vpn.serverData();
    Q_ASSERT(sd);
    // Now we need to select a server.
    Server exitServer = Server::weightChooser(sd->exitServers());
    exitConfig.m_hopType = InterfaceConfig::SingleHop;
    exitConfig.m_privateKey = devicePrivateKey;
    exitConfig.m_deviceIpv4Address = deviceIpv4Address;
    exitConfig.m_deviceIpv6Address = deviceIpv6Address;
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
        entryConfig.m_privateKey = devicePrivateKey;
        entryConfig.m_deviceIpv4Address = deviceIpv4Address;
        entryConfig.m_deviceIpv6Address = deviceIpv6Address;
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
        // configure main peer for Mullvad multihop
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
    stream << newDeviceComment;
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
      stream << "# --mullvad-multihop flag.\n\n";
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
