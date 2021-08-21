/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandselect.h"
#include "leakdetector.h"
#include "localizer.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

#include <QTextStream>

CommandSelect::CommandSelect(QObject* parent)
    : Command(parent, "select", "Select a server.") {
  MVPN_COUNT_CTOR(CommandSelect);
}

CommandSelect::~CommandSelect() { MVPN_COUNT_DTOR(CommandSelect); }

int CommandSelect::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return runCommandLineApp([&]() {
    if (tokens.length() != 2) {
      QTextStream stream(stdout);
      stream << "usage: " << tokens[0] << " <server_hostname>" << Qt::endl;
      stream << Qt::endl;
      stream << "The list of <server_hostname>s can be obtained using: '"
             << tokens[0].split(" ").at(0) << " servers'" << Qt::endl;
      return 1;
    }

    if (!userAuthenticated()) {
      return 1;
    }

    MozillaVPN vpn;
    if (!loadModels()) {
      return 1;
    }

    ServerData sd;
    if (!pickServer(tokens[1], sd)) {
      QTextStream stream(stdout);
      stream << "unknown server hostname: " << tokens[1] << Qt::endl;
      return 1;
    }

    vpn.changeServer(sd.countryCode(), sd.cityName());
    return 0;
  });
}

bool CommandSelect::pickServer(const QString& hostname,
                               ServerData& serverData) {
  for (const ServerCountry& country :
       MozillaVPN::instance()->serverCountryModel()->countries()) {
    for (const ServerCity& city : country.cities()) {
      for (const Server& server : city.servers()) {
        if (server.hostname() == hostname) {
          serverData.update(country.code(), city.name());
          return true;
        }
      }
    }
  }

  return false;
}

static Command::RegistrationProxy<CommandSelect> s_commandSelect;
