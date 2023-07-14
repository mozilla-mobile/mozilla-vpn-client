/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandexcludeip.h"

#include <QHostAddress>
#include <QTextStream>

#include "ipaddress.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include "settingsholder.h"

constexpr const int LIST = 0;
constexpr const int ADD = 1;
constexpr const int REMOVE = 2;

CommandExcludeIP::CommandExcludeIP(QObject* parent)
    : Command(parent, "excludeip", "Exclude IP addresses from VPN routing") {
  MZ_COUNT_CTOR(CommandExcludeIP);
}

CommandExcludeIP::~CommandExcludeIP() { MZ_COUNT_DTOR(CommandExcludeIP); }

int CommandExcludeIP::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    QStringList commandList{"list", "add", "remove"};

    qsizetype command = -1;
    if (tokens.length() > 1) {
      command = commandList.indexOf(tokens[1]);
    }

    QTextStream stream(stdout);
    if (command < 0) {
      stream << "usage: " << tokens[0] << " <list|add|remove> [ip]" << Qt::endl;
      return 1;
    }
    if (command == LIST && tokens.length() != 2) {
      stream << "usage: " << tokens[0] << " " << tokens[1] << Qt::endl;
      return 1;
    }
    if (command != LIST && tokens.length() != 3) {
      stream << "usage: " << tokens[0] << " " << tokens[1] << " <ip>"
             << Qt::endl;
      stream << Qt::endl;
      stream << "The list of excluded <ips> can be obtained using: 'list'"
             << Qt::endl;
      return 1;
    }

    if (!userAuthenticated()) {
      return 1;
    }

    MozillaVPN vpn;
    if (!loadModels()) {
      return 1;
    }

    SettingsHolder* settingsHolder = SettingsHolder::instance();
    QStringList ipv4Addresses = settingsHolder->excludedIpv4Addresses();
    QStringList ipv6Addresses = settingsHolder->excludedIpv6Addresses();

    if (command == LIST) {
      stream << "Excluded IP v4 addresses:" << Qt::endl;
      for (const QString& ipv4String : ipv4Addresses) {
        stream << "- " << ipv4String << Qt::endl;
      }
      stream << "Excluded IP v6 addresses:" << Qt::endl;
      for (const QString& ipv6String : ipv6Addresses) {
        stream << "- " << ipv6String << Qt::endl;
      }
      return 0;
    }

    // parseSubnet allows generous parsing, also works on IPs without /prefix
    QHostAddress ip;
    int prefix;
    std::tie(ip, prefix) = QHostAddress::parseSubnet(tokens[2]);

    if (ip.isNull() || prefix < 0) {
      stream << tokens[2] << " is not a valid IP address." << Qt::endl;
      return 1;
    }

    // Use IPAddress to get a canonical string representation
    QString ipString = IPAddress(ip, prefix).toString();
    if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
      if (command == ADD && !ipv4Addresses.contains(ipString)) {
        ipv4Addresses << ipString;
      } else if (command == REMOVE && ipv4Addresses.removeAll(ipString) == 0) {
        stream << ipString << " is not currently an excluded IP address."
               << Qt::endl;
        return 1;
      }
      settingsHolder->setExcludedIpv4Addresses(ipv4Addresses);
    } else if (ip.protocol() == QAbstractSocket::IPv6Protocol) {
      if (command == ADD && !ipv6Addresses.contains(ipString)) {
        ipv6Addresses << ipString;
      } else if (command == REMOVE && ipv6Addresses.removeAll(ipString) == 0) {
        stream << ipString << " is not currently an excluded IP address."
               << Qt::endl;
        return 1;
      }
      settingsHolder->setExcludedIpv6Addresses(ipv6Addresses);
    } else {
      Q_ASSERT(false);
    }

    return 0;
  });
}

static Command::RegistrationProxy<CommandExcludeIP> s_commandDevice;
