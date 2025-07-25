/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandselect.h"

#include <QTextStream>

#include "leakdetector.h"
#include "models/servercity.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "mozillavpn.h"

CommandSelect::CommandSelect(QObject* parent)
    : Command(parent, "select", "Select a server.") {
  MZ_COUNT_CTOR(CommandSelect);
}

CommandSelect::~CommandSelect() { MZ_COUNT_DTOR(CommandSelect); }

int CommandSelect::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());
  return MozillaVPN::runCommandLineApp([&]() {
    QTextStream stream(stdout);
    if ((tokens.length() < 2) || (tokens.length() > 3)) {
      stream << "usage: " << tokens[0] << " <server_hostname> [entry_hostname]"
             << Qt::endl;
      stream << Qt::endl;
      stream << "The list of hostnames can be obtained using: '"
             << tokens[0].split(" ").at(0) << " servers'" << Qt::endl;
      return 1;
    }

    MozillaVPN vpn;
    if (!vpn.hasToken()) {
      stream << "User status: not authenticated" << Qt::endl;
      return 1;
    }
    if (!vpn.loadModels()) {
      stream << "No cache available" << Qt::endl;
      return 1;
    }

    QString exitCountryCode;
    QString exitCityName;
    QString entryCountryCode;
    QString entryCityName;
    if (!pickServer(tokens[1], exitCountryCode, exitCityName)) {
      stream << "unknown server hostname: " << tokens[1] << Qt::endl;
      return 1;
    }

    if ((tokens.length() > 2) &&
        !pickServer(tokens[2], entryCountryCode, entryCityName)) {
      stream << "unknown server hostname: " << tokens[2] << Qt::endl;
      return 1;
    }

    vpn.serverData()->changeServer(exitCountryCode, exitCityName,
                                   entryCountryCode, entryCityName);
    return 0;
  });
}

bool CommandSelect::pickServer(const QString& hostname, QString& countryCode,
                               QString& cityName) {
  ServerCountryModel* model = MozillaVPN::instance()->serverCountryModel();
  for (const ServerCountry& country : model->countries()) {
    for (const QString& name : country.cities()) {
      const ServerCity& city = model->findCity(country.code(), name);
      if (!city.initialized()) {
        continue;
      }
      for (const QString& pubkey : city.servers()) {
        const Server& server = model->server(pubkey);
        if (server.hostname() == hostname) {
          countryCode = country.code();
          cityName = city.name();
          return true;
        }
      }
    }
  }

  return false;
}

static Command::RegistrationProxy<CommandSelect> s_commandSelect;
