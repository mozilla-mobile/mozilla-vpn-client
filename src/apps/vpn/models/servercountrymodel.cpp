/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercountrymodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "appconstants.h"
#include "collator.h"
#include "feature.h"
#include "leakdetector.h"
#include "location.h"
#include "logger.h"
#include "servercountry.h"
#include "serverdata.h"
#include "serveri18n.h"
#include "settingsholder.h"

namespace {
Logger logger("ServerCountryModel");
}

ServerCountryModel::ServerCountryModel() { MZ_COUNT_CTOR(ServerCountryModel); }

ServerCountryModel::~ServerCountryModel() { MZ_COUNT_DTOR(ServerCountryModel); }

bool ServerCountryModel::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading the server list from settings";

  const QByteArray json = settingsHolder->servers();
  if (json.isEmpty() || !fromJsonInternal(json)) {
    return false;
  }

  m_rawJson = json;
  return true;
}

bool ServerCountryModel::fromJson(const QByteArray& s) {
  logger.debug() << "Reading from JSON";

  if (!s.isEmpty() && m_rawJson == s) {
    logger.debug() << "Nothing has changed";
    return true;
  }

  if (!fromJsonInternal(s)) {
    return false;
  }

  m_rawJson = s;
  emit changed();
  return true;
}

bool ServerCountryModel::fromJsonInternal(const QByteArray& s) {
  beginResetModel();

  m_rawJson = "";
  m_countries.clear();
  m_servers.clear();

  QJsonDocument doc = QJsonDocument::fromJson(s);
  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  QJsonValue countries = obj.value("countries");
  if (!countries.isArray()) {
    return false;
  }

  QJsonArray countriesArray = countries.toArray();
  for (const QJsonValue& countryValue : countriesArray) {
    if (!countryValue.isObject()) {
      return false;
    }

    QJsonObject countryObj = countryValue.toObject();

    ServerCountry country;
    if (!country.fromJson(countryObj)) {
      return false;
    }

    if (country.cities().isEmpty()) {
      continue;
    }

    m_countries.append(country);

    QJsonValue cities = countryObj.value("cities");
    if (!cities.isArray()) {
      return false;
    }

    QJsonArray cityArray = cities.toArray();
    for (const QJsonValue& cityValue : cityArray) {
      if (!cityValue.isObject()) {
        return false;
      }
      QJsonObject cityObj = cityValue.toObject();
      QJsonValue servers = cityObj.value("servers");
      if (!servers.isArray()) {
        return false;
      }

      QJsonArray serverArray = servers.toArray();
      for (const QJsonValue& serverValue : serverArray) {
        Server server;
        if (!server.fromJson(serverValue.toObject())) {
          return false;
        }
        m_servers[server.publicKey()] = server;
      }
    }
  }

  sortCountries();

  endResetModel();

  return true;
}

QHash<int, QByteArray> ServerCountryModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[LocalizedNameRole] = "localizedName";
  roles[CodeRole] = "code";
  roles[CitiesRole] = "cities";
  return roles;
}

QVariant ServerCountryModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() >= m_countries.length()) {
    return QVariant();
  }

  switch (role) {
    case NameRole:
      return QVariant(m_countries.at(index.row()).name());

    case LocalizedNameRole: {
      const ServerCountry& country = m_countries.at(index.row());
      return QVariant(
          ServerI18N::translateCountryName(country.code(), country.name()));
    }

    case CodeRole:
      return QVariant(m_countries.at(index.row()).code());

    case CitiesRole: {
      const ServerCountry& country = m_countries.at(index.row());

      QList<QVariant> list;
      for (const ServerCity& city : country.cities()) {
        list.append(QVariant::fromValue(&city));
      }

      return QVariant(list);
    }

    default:
      return QVariant();
  }
}

int ServerCountryModel::cityConnectionScore(const QString& countryCode,
                                            const QString& cityCode) const {
  for (const ServerCountry& country : m_countries) {
    if (country.code() != countryCode) {
      continue;
    }

    for (const ServerCity& city : country.cities()) {
      if (city.code() != cityCode) {
        continue;
      }

      return cityConnectionScore(city);
    }

    // No such city was found.
    return NoData;
  }

  // No such country was found.
  return NoData;
}

int ServerCountryModel::cityConnectionScore(const ServerCity& city) const {
  qint64 now = QDateTime::currentSecsSinceEpoch();
  int score = Poor;
  int activeServerCount = 0;
  uint32_t sumLatencyMsec = 0;
  for (const QString& pubkey : city.servers()) {
    const Server& server = m_servers[pubkey];
    if (server.cooldownTimeout() <= now) {
      sumLatencyMsec += server.latency();
      activeServerCount++;
    }
  }

  // Ensure there is at least one reachable server.
  if (activeServerCount == 0) {
    return Unavailable;
  }

  // If the feature is disabled, we have no data to return.
  if (!Feature::get(Feature::Feature_serverConnectionScore)->isSupported()) {
    return NoData;
  }
  // In the unlikely event that the sum of the latencies is zero, then we
  // haven't actually measured anything and have nothing to report.
  if (sumLatencyMsec == 0) {
    return NoData;
  }

  // Increase the score if the location has less than 100ms of latency.
  if ((sumLatencyMsec / activeServerCount) < 100) {
    score++;
  }

  // Increase the score if the location has 6 or more servers.
  if (activeServerCount >= 6) {
    score++;
  }

  if (score > Good) {
    score = Good;
  }
  return score;
}

QStringList ServerCountryModel::pickRandom() const {
  logger.debug() << "Choosing a random server";
  qsizetype index = QRandomGenerator::global()->generate() % m_servers.count();

  // Iterate to find the selected country and city. This winds up weighting the
  // choice of city proportional to the number of servers hosted there.
  for (auto country = m_countries.cbegin(); country != m_countries.cend();
       country++) {
    for (auto city = country->cities().cbegin();
         city != country->cities().cend(); city++) {
      if (index >= city->servers().count()) {
        // Keep searching.
        index -= city->servers().count();
      } else {
        // We found our selection.
        QStringList serverChoice = {
            country->code(), city->name(),
            ServerI18N::translateCityName(country->code(), city->name())};
        return serverChoice;
      }
    }
  }

  // We should not get here, unless the model has more entries in m_servers()
  // than actually exist in the country and city lists.
  Q_ASSERT(false);
  return QStringList();
}

// Select the city that we think is going to perform the best
QStringList ServerCountryModel::pickBest(const Location& location) const {
  double latitude = location.latitude();
  double longitude = location.longitude();
  if (qIsNaN(latitude) || qIsNaN(longitude)) {
    // If we don't know the client's location, just pick at random.
    return pickRandom();
  }

  // We rank cities using the distance between two points on a great circle,
  // which is given by:
  //    d = acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(long1-long2))
  //
  // TODO: Include other ranking data, such as latency and number of servers.
  QString bestCountry;
  QString bestCity;
  double clientSin = qSin(latitude * M_PI / 180.0);
  double clientCos = qCos(latitude * M_PI / 180.0);
  double bestDistance = M_2_PI;
  for (const ServerCountry& country : m_countries) {
    for (const ServerCity& city : country.cities()) {
      double citySin = qSin(city.latitude() * M_PI / 180.0);
      double cityCos = qCos(city.latitude() * M_PI / 180.0);
      double diffCos = qCos((city.longitude() - longitude) * M_PI / 180.0);
      double distance =
          qAcos(clientSin * citySin + clientCos * cityCos * diffCos);

      if (distance < bestDistance) {
        bestCountry = country.code();
        bestCity = city.name();
        bestDistance = distance;
      }
    }
  }

  if (bestCountry.isEmpty() || bestCity.isEmpty()) {
    return pickRandom();
  }
  return QStringList({bestCountry, bestCity});
}

bool ServerCountryModel::exists(const QString& countryCode,
                                const QString& cityName) const {
  logger.debug() << "Check if the server is still valid.";

  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      for (const ServerCity& city : country.cities()) {
        if (cityName == city.name()) {
          return true;
        }
      }

      break;
    }
  }

  return false;
}

const QList<Server> ServerCountryModel::servers(const QString& countryCode,
                                                const QString& cityName) const {
  QList<Server> results;

  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      for (const QString& pubkey : country.serversFromCityName(cityName)) {
        if (m_servers.contains(pubkey)) {
          results.append(m_servers.value(pubkey));
        }
      }
    }
  }

  return results;
}

const QString ServerCountryModel::countryName(
    const QString& countryCode) const {
  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      return country.name();
    }
  }

  return QString();
}

void ServerCountryModel::retranslate() {
  beginResetModel();
  sortCountries();
  endResetModel();
}

void ServerCountryModel::setServerLatency(const QString& publicKey,
                                          unsigned int msec) {
  if (m_servers.contains(publicKey)) {
    m_servers[publicKey].setLatency(msec);
  }
}

void ServerCountryModel::setServerCooldown(const QString& publicKey) {
  if (m_servers.contains(publicKey)) {
    m_servers[publicKey].setCooldownTimeout(
        AppConstants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);
  }
}

void ServerCountryModel::setCooldownForAllServersInACity(
    const QString& countryCode, const QString& cityCode) {
  logger.debug() << "Set cooldown for all servers for: "
                 << logger.sensitive(countryCode) << logger.sensitive(cityCode);

  for (const ServerCountry& country : m_countries) {
    if (country.code() == countryCode) {
      for (const ServerCity& city : country.cities()) {
        if (city.code() == cityCode) {
          for (const QString& pubkey : city.servers()) {
            setServerCooldown(pubkey);
          }
          break;
        }
      }
      break;
    }
  }
}

namespace {

bool sortCountryCallback(const ServerCountry& a, const ServerCountry& b,
                         Collator* collator) {
  Q_ASSERT(collator);
  return collator->compare(
             ServerI18N::translateCountryName(a.code(), a.name()),
             ServerI18N::translateCountryName(b.code(), b.name())) < 0;
}

}  // anonymous namespace

void ServerCountryModel::sortCountries() {
  Collator collator;
  std::sort(m_countries.begin(), m_countries.end(),
            std::bind(sortCountryCallback, std::placeholders::_1,
                      std::placeholders::_2, &collator));

  for (ServerCountry& country : m_countries) {
    country.sortCities();
  }
}
